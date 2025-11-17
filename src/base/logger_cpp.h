#include "base_inc.h"

#include <stdarg.h>

struct LoggerSystemState {
  OS_Handle log_file_handle;
};

global LoggerSystemState st;

intern void append_to_log_file(String message) {
  os_file_write(st.log_file_handle, message.size, message.str);
}

void _log_output(LogLevel level, String fmt, ...) {
  Scratch scratch;
  String level_strings[] = {"[TRACE]: ", "[DEBUG]: ", "[INFO]:  ", "[WARN]:  ", "[ERROR]: ",};
  va_list argc;
  va_start(argc, fmt);
  String formatted = push_strfv(scratch, fmt, argc);
  va_end(argc);
  String out_message = push_strf(scratch, "%s%s\n", level_strings[level], formatted);
  os_console_write(out_message, level);
}

void print(String fmt, ...) {
  Scratch scratch;
  va_list argc;
  va_start(argc, fmt);
  String formatted = push_strfv(scratch, fmt, argc);
  va_end(argc);
  String out_message = push_strf(scratch, "%s", formatted);
  os_console_write(out_message, -1);
}

void println(String fmt, ...) {
  Scratch scratch;
  va_list argc;
  va_start(argc, fmt);
  String formatted = push_strfv(scratch, fmt, argc);
  va_end(argc);
  String out_message = push_strf(scratch, "%s\n", formatted);
  os_console_write(out_message, -1);
}
