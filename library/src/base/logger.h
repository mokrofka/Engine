#pragma once

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

#if KRELEASE == 1
  #define LOG_DEBUG_ENABLED 0
  #define LOG_TRACED_ENABLED 0
#endif

enum LogLevel {
  LOG_LEVEL_FATAL = 0,
  LOG_LEVEL_ERROR = 1,
  LOG_LEVEL_WARN = 2,
  LOG_LEVEL_INFO = 3,
  LOG_LEVEL_DEBUG = 4,
  LOG_LEVEL_TRACE = 5,
};

KAPI void _log_output(LogLevel level, String message);
KAPI void _log_output(LogLevel level, const void* fmt, ...);

// Logs a fatal-level message.
#define Fatal(message, ...) _log_output(LOG_LEVEL_FATAL, message, ##__VA_ARGS__)

#ifndef KERROR
// Logs an error-level message.
#define Error(message, ...) _log_output(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
// Logs a warning-level message.
#define Warn(message, ...) _log_output(LOG_LEVEL_WARN, message, ##__VA_ARGS__)
#else
// Does nothing when LOG_WARN_ENABLED != 1
#define Warn(message, ...);
#endif

#if LOG_INFO_ENABLED == 1
// Logs a info-level message.
#define Info(message, ...) _log_output(LOG_LEVEL_INFO, message, ##__VA_ARGS__)

#else
// Does nothing when LOG_INFO_ENABLED != 1
#define Info(message, ...);
#endif

#if LOG_DEBUG_ENABLED == 1
// Logs a debug-level message.
#define Debug(message, ...) _log_output(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__)
#else
// Does nothing when LOG_DEBUG_ENABLED != 1
#define Debug(message, ...);
#endif

#if LOG_TRACE_ENABLED == 1
// Logs a trace-level message.
#define Trace(message, ...) _log_output(LOG_LEVEL_TRACE, message, ##__VA_ARGS__)
#else
// Does nothing when LOG_TRACE_ENABLED != 1
#define Trace(message, ...);
#endif


// Assert
// Disable assertsions by commenting out the below line.
#define KASSERTIONS_ENABLED

#ifdef KASSERTIONS_ENABLED
#if _MSC_VER
  #define debugBreak() __debugbreak();
#else
  #define debugBreak() __builtin_trap();
#endif

#define Assert(expr)                                           \
  {                                                            \
    if (expr) {                                                \
    } else {                                                   \
      debugBreak();                                            \
    }                                                          \
  }

#else
  #define Assert(expr)              // Does nothing at all
#endif
