#pragma once
#include "defines.h"
#include "str.h"

#define LOG_TRACE_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_WARN_ENABLED 1
#define LOG_ERROR_ENABLED 1

#define ASSERTIONS_ENABLED 1

enum LogLevel {
  LogLevel_Trace,
  LogLevel_Debug,
  LogLevel_Info,
  LogLevel_Warn,
  LogLevel_Error,
};

KAPI void _log_output(LogLevel level, String fmt, ...); // with \n
KAPI void print(String fmt, ...);
KAPI void println(String fmt, ...);

#if LOG_TRACE_ENABLED
  #define Trace(message, ...) _log_output(LogLevel_Trace, message, ##__VA_ARGS__)
#else
  #define Trace(message, ...)
#endif

#if LOG_DEBUG_ENABLED
  #define Debug(message, ...) _log_output(LogLevel_Debug, message, ##__VA_ARGS__)
#else
  #define Debug(message, ...)
#endif

#if LOG_INFO_ENABLED
  #define Info(message, ...) _log_output(LogLevel_Info, message, ##__VA_ARGS__)
#else
  #define Info(message, ...)
#endif

#if LOG_WARN_ENABLED
  #define Warn(message, ...) _log_output(LogLevel_Warn, message, ##__VA_ARGS__)
#else
  #define Warn(message, ...)
#endif

#if LOG_ERROR_ENABLED
  #define Error(message, ...) _log_output(LogLevel_Error, message, ##__VA_ARGS__);
#else
  #define Error(message, ...)
#endif

#if ASSERTIONS_ENABLED
  #define Assert(expr)  \
    {                   \
      if (expr) {       \
      } else {          \
        Error("Assert") \
        DebugBreak();   \
      }                 \
    }

  #define AssertMsg(expr, message, ...)                      \
    {                                                        \
      if (expr) {                                            \
      } else {                                               \
        _log_output(LogLevel_Error, message, ##__VA_ARGS__); \
        DebugBreak();                                        \
      }                                                      \
    }

#else
  #define Assert(expr)
  #define AssertMsg(expr)
#endif
