#include "lib.h"

#include <stdarg.h>

struct LoggerSystemState {
  OS_Handle log_file_handle;
};

global LoggerSystemState* state;

internal void append_to_log_file(String message) {
  os_file_write(state->log_file_handle, message.size, message.str);
}

void _log_output(LogLevel level, String message) {
  Scratch scratch;
  String level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
  b32 error = level < 2;

  String out_message = push_strf(scratch, "%s%s\n", level_strings[level], message);

  if (error) {
    os_console_write_error(out_message, level);
  } else {
    os_console_write(out_message, level);
  }

  // Queue a copy to be written to the log file
  // append_to_log_file(out_message);
}

void _log_output(LogLevel level, const void* fmt, ...) {
  Scratch scratch;
  String level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
  b32 error = level < 2;

  va_list argc;
  va_start(argc, fmt);
  String formatted = push_strfv(scratch, fmt, argc);
  va_end(argc);

  String out_message = push_strf(scratch, "%s%s\n", level_strings[level], formatted);

  if (error) {
    os_console_write_error(out_message, level);
  } else {
    os_console_write(out_message, level);
  }

  // Queue a copy to be written to the log file
  // append_to_log_file(out_message);
}
