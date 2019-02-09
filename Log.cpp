#include "Log.h"
#include "mcpelauncher_api.h"

void Log::vlog(LogLevel level, const char* tag, const char* text, va_list args) {
    mcpelauncher_vlog((mcpelauncher_log_level) level, tag, text, args);
}