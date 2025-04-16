#include "logger.h"
#include "os.h"
#include "memory.h"
#include "str.h"

#include <stdarg.h>

struct LoggerSystemState {
  OS_Handle log_file_handle;
};

global LoggerSystemState* state;

internal void append_to_log_file(String message) {
  os_file_write(state->log_file_handle, message.size, message.str);
}

void logging_init(Arena* arena) {
  state = push_struct(arena, LoggerSystemState);
  
  state->log_file_handle = os_file_open(str_lit("console.log"), OS_AccessFlag_Write);
  Assert(state->log_file_handle.u64);
}

void shutdown_logging() {
  // TODO: cleanup logging/write queued entries.
}

void _log_output(LogLevel level, const char* message, ...) {
  Scratch scratch;
  char* level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
  b32 error = level < 2;

  va_list argc;
  va_start(argc, message);
  String formatted = push_strfv(scratch, message, argc);
  va_end(argc);

  String out_message = push_strf(scratch, "%s%s\n", level_strings[level], formatted.str);

  if (error) {
    os_console_write_error(out_message, level);
  } else {
    os_console_write(out_message, level);
  }

  // Queue a copy to be written to the log file
  append_to_log_file(out_message);
}

void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line) {
  _log_output(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s', in file: %s, "
              "line: %d\n", expression, message, file, line);
}
