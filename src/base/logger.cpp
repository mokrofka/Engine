#include "logger.h"
#include "thread_ctx.h"
#include "os/os_core.h"

void _log_output(LogLevel level, String fmt, ...) {
  Scratch scratch;
  String level_strings[] = {"[TRACE]: ", "[DEBUG]: ", "[INFO]:  ", "[WARN]:  ", "[ERROR]: ",};
  VaList argc;
  va_start(argc, fmt);
  String formatted = push_strfv(scratch, fmt, argc);
  va_end(argc);
  String out_message = push_strf(scratch, "%s%s\n", level_strings[level-1], formatted);
  os_console_write(out_message, level);
}

void _log_output(Allocator arena, LogLevel level, String fmt, ...) {
  String level_strings[] = {"[TRACE]: ", "[DEBUG]: ", "[INFO]:  ", "[WARN]:  ", "[ERROR]: ",};
  VaList args;
  va_start(args, fmt);
  String formatted = push_strfv(arena, fmt, args);
  va_end(args);
  String out_message = push_strf(arena, "%s%s\n", level_strings[level-1], formatted);
  os_console_write(out_message, level);
}

void print(String fmt, ...) {
  Scratch scratch;
  VaList argc;
  va_start(argc, fmt);
  String formatted = push_strfv(scratch, fmt, argc);
  va_end(argc);
  String out_message = push_strf(scratch, "%s", formatted);
  os_console_write(out_message, 0);
}

void println(String fmt, ...) {
  Scratch scratch;
  VaList argc;
  va_start(argc, fmt);
  String formatted = push_strfv(scratch, fmt, argc);
  va_end(argc);
  String out_message = push_strf(scratch, "%s\n", formatted);
  os_console_write(out_message, 0);
}
