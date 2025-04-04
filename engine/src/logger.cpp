#include "logger.h"
#include "os.h"
#include "memory.h"
#include "str.h"

#include <stdarg.h>

struct LoggerSystemState {
  OS_File log_file_handle;
};

global LoggerSystemState* state;

internal void append_to_log_file(const char* message) {
  if (state && state->log_file_handle.u64) {
    u64 length = cstr_length((u8*)message);
    u64 written = 0;
    if (!os_file_write(state->log_file_handle, length, message)) {
      Error("writing to console.log");
    }
  }
}

b8 logging_init(Arena* arena) {
  state = push_struct(arena, LoggerSystemState);
  
  state->log_file_handle = os_file_open(str_lit("console.log"), FILE_MODE_WRITE);
  if (!state->log_file_handle.u64) {
    Error("Unable to open console.log for writing.");
    return false;
  }
  
  return true;
}

void shutdown_logging() {
  // TODO: cleanup logging/write queued entries.
}

void log_output(LogLevel level, const char* message, ...) {
  const char* level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
  b8 is_error = level < 2;

  {
    Scratch scratch;
    u8* formatted = push_buffer(scratch.arena, u8, 32000);
    u8* out_message = push_buffer(scratch.arena, u8, 32000);

    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    str_format_v(formatted, message, arg_ptr);
    va_end(arg_ptr);

    str_format(out_message, "%s%s\n", level_strings[level], formatted);

    if (is_error) {
      os_console_write_error((char*)out_message, level);
    } else {
      os_console_write((char*)out_message, level);
    }

    // if (level == LOG_LEVEL_FATAL) {
    //   debugBreak();
    // }

    // Queue a copy to be written to the log file
    append_to_log_file((char*)out_message);
  }
}

void 
report_assertion_failure(const char* expression, const char* message, const char* file, i32 line) {
    log_output(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s', in file: %s, "
               "line: %d\n", expression, message, file, line);
}
