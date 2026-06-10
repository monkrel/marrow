#include "app/logging.hpp"
#include "core/version.hpp"

#include <spdlog/spdlog.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include <windows.h>

#include <d3d11.h>

namespace {

ID3D11Device* g_device = nullptr;
ID3D11DeviceContext* g_device_context = nullptr;
IDXGISwapChain* g_swap_chain = nullptr;
ID3D11RenderTargetView* g_render_target_view = nullptr;
bool g_swap_chain_occluded = false;
UINT g_resize_width = 0;
UINT g_resize_height = 0;

void create_render_target() {
    ID3D11Texture2D* back_buffer = nullptr;
    if (SUCCEEDED(g_swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer))) && back_buffer) {
        g_device->CreateRenderTargetView(back_buffer, nullptr, &g_render_target_view);
        back_buffer->Release();
    }
}

void cleanup_render_target() {
    if (g_render_target_view) {
        g_render_target_view->Release();
        g_render_target_view = nullptr;
    }
}

bool create_device_d3d(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    constexpr D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };
    D3D_FEATURE_LEVEL created_level = {};
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, feature_levels, 2, D3D11_SDK_VERSION,
        &sd, &g_swap_chain, &g_device, &created_level, &g_device_context);
    if (hr == DXGI_ERROR_UNSUPPORTED) {
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, feature_levels, 2, D3D11_SDK_VERSION,
            &sd, &g_swap_chain, &g_device, &created_level, &g_device_context);
    }
    if (FAILED(hr)) {
        return false;
    }
    create_render_target();
    return true;
}

void cleanup_device_d3d() {
    cleanup_render_target();
    if (g_swap_chain) {
        g_swap_chain->Release();
        g_swap_chain = nullptr;
    }
    if (g_device_context) {
        g_device_context->Release();
        g_device_context = nullptr;
    }
    if (g_device) {
        g_device->Release();
        g_device = nullptr;
    }
}

} // namespace

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam,
                                                             LPARAM lparam);

namespace {

LRESULT WINAPI wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
        return 1;
    }
    switch (msg) {
    case WM_SIZE:
        if (wparam != SIZE_MINIMIZED) {
            g_resize_width = LOWORD(lparam);
            g_resize_height = HIWORD(lparam);
        }
        return 0;
    case WM_SYSCOMMAND:
	//ALT key
        if ((wparam & 0xFFF0) == SC_KEYMENU) {
            return 0;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
    marrow::app::init_logging();
    spdlog::info("{} {} starting", marrow::kAppName, marrow::app_version());

    ImGui_ImplWin32_EnableDpiAwareness();

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = L"MarrowMainWindow";
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"Marrow", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 1280, 800, nullptr, nullptr,
                                instance, nullptr);
    if (!hwnd || !create_device_d3d(hwnd)) {
        spdlog::critical("Direct3D 11 init failed; exiting");
        cleanup_device_d3d();
        UnregisterClassW(wc.lpszClassName, instance);
        spdlog::shutdown();
        return 1;
    }
    spdlog::info("Direct3D 11 device and swap chain created");

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_device, g_device_context);

    bool show_demo_window = false;
    bool done = false;
    while (!done) {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_QUIT) {
                done = true;
            }
        }
        if (done) {
            break;
        }

        // Skip rendering if window is minimized or hidden
        if (g_swap_chain_occluded &&
            g_swap_chain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
            Sleep(10);
            continue;
        }
        g_swap_chain_occluded = false;

        if (g_resize_width != 0 && g_resize_height != 0) {
            cleanup_render_target();
            g_swap_chain->ResizeBuffers(0, g_resize_width, g_resize_height, DXGI_FORMAT_UNKNOWN,
                                        0);
            g_resize_width = 0;
            g_resize_height = 0;
            create_render_target();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Exit", "Alt+F4")) {
                    done = true;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                ImGui::MenuItem("Dear ImGui Demo", nullptr, &show_demo_window);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        ImGui::Begin("Shell");
        ImGui::Text("%s %s", marrow::kAppName.data(), marrow::app_version().data());
        ImGui::Text("Frame time %.3f ms (%.1f FPS)", 1000.0 / static_cast<double>(io.Framerate),
                    static_cast<double>(io.Framerate));
        ImGui::End();

        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        ImGui::Render();
        constexpr float clear_color[4] = {0.10f, 0.10f, 0.12f, 1.00f};
        g_device_context->OMSetRenderTargets(1, &g_render_target_view, nullptr);
        g_device_context->ClearRenderTargetView(g_render_target_view, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        const HRESULT present_result = g_swap_chain->Present(1, 0);
        g_swap_chain_occluded = present_result == DXGI_STATUS_OCCLUDED;
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    cleanup_device_d3d();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, instance);

    spdlog::info("Clean shutdown");
    spdlog::shutdown();
    return 0;
}
