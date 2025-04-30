#include "logger.h"
#include "os.h"
#include "memory.h"
#include "strings.h"

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
  
  // state->log_file_handle = os_file_open("console.log"_, OS_AccessFlag_Write);
  // Assert(state->log_file_handle.u64);
}

void shutdown_logging() {
  // TODO: cleanup logging/write queued entries.
}

void _log_output(LogLevel level, String message) {
  Scratch scratch;
  String level_strings[6] = {"[FATAL]: "_, "[ERROR]: "_, "[WARN]:  "_, "[INFO]:  "_, "[DEBUG]: "_, "[TRACE]: "_};
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
  String level_strings[6] = {"[FATAL]: "_, "[ERROR]: "_, "[WARN]:  "_, "[INFO]:  "_, "[DEBUG]: "_, "[TRACE]: "_};
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
