#include "logger.h"
#include "asserts.h"
#include "platform/platform.h"
#include "memory.h"
#include "strings.h"

#include <stdarg.h>

struct LoggerSystemState {

};

b8 initialize_logging() {
  // TODO: create log file.
  return true;
}

void shutdown_logging() {
  // TODO: cleanup logging/write queued entries.
}

void log_output(LogLevel level, const char* message, ...) {
  const char* level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
  b8 is_error = level < LOG_LEVEL_WARN;
  
  {
    Temp scratch = GetScratch(0, 0);
    u8* formatted = push_buffer(scratch, u8, 32000);
    u8* out_message = push_buffer(scratch, u8, 32000);

    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    str_format_v(formatted, message, arg_ptr);
    va_end(arg_ptr);

    str_format(out_message, "%s%s\n", level_strings[level], formatted);

    // platform-specific output.
    if (is_error) {
      platform_console_write_error((char*)out_message, level);
    } else {
      platform_console_write((char*)out_message, level);
    }
    ReleaseScratch(scratch);
  }
  
  if (level == LOG_LEVEL_FATAL) {
    debugBreak();
  }
}

void 
report_assertion_failure(const char* expression, const char* message, const char* file, i32 line) {
    log_output(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s', in file: %s, "
               "line: %d\n", expression, message, file, line);
}
