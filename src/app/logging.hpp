#pragma once

namespace marrow::app {

// Sets up default logging to %LOCALAPPDATA%\Marrow\Logs,
// falling back to debug output if file logging is unavailable.
void init_logging();

} // namespace marrow::app
