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

KAPI void _log_output(LogLevel level, String fmt, ...);        // with \n
KAPI void _log_output_inline(LogLevel level, String fmt, ...); // without \n
KAPI void _log_output_raw(LogLevel level, String fmt, ...);    // without tag
KAPI void print(String fmt, ...);
KAPI void println(String fmt, ...);

#if LOG_TRACE_ENABLED
  #define Trace(message, ...) _log_output(LogLevel_Trace, message, ##__VA_ARGS__)
  #define TraceInline(message, ...) _log_output_inline(LogLevel_Trace, message, ##__VA_ARGS__)
  #define TraceRaw(message, ...) _log_output_raw(LogLevel_Trace, message, ##__VA_ARGS__)
#else
  #define Trace(message, ...)
  #define TraceInline(message, ...)
  #define TraceRaw(message, ...)
#endif

#if LOG_DEBUG_ENABLED
  #define Debug(message, ...) _log_output(LogLevel_Debug, message, ##__VA_ARGS__)
  #define DebugInline(message, ...) _log_output_inline(LogLevel_Debug, message, ##__VA_ARGS__)
  #define DebugRaw(message, ...) _log_output_raw(LogLevel_Debug, message, ##__VA_ARGS__)
#else
  #define Debug(message, ...)
  #define DebugInline(message, ...)
  #define DebugRaw(message, ...)
#endif

#if LOG_INFO_ENABLED
  #define Info(message, ...) _log_output(LogLevel_Info, message, ##__VA_ARGS__)
  #define InfoInline(message, ...) _log_output_inline(LogLevel_Info, message, ##__VA_ARGS__)
  #define InfoRaw(message, ...) _log_output_raw(LogLevel_Info, message, ##__VA_ARGS__)
#else
  #define Info(message, ...)
  #define InfoInline(message, ...)
  #define InfoRaw(message, ...)
#endif

#if LOG_WARN_ENABLED
  #define Warn(message, ...) _log_output(LogLevel_Warn, message, ##__VA_ARGS__)
  #define WarnInline(message, ...) _log_output_inline(LogLevel_Warn, message, ##__VA_ARGS__)
  #define WarnRaw(message, ...) _log_output_raw(LogLevel_Warn, message, ##__VA_ARGS__)
#else
  #define Warn(message, ...)
  #define WarnInline(message, ...)
  #define WarnRaw(message, ...)
#endif

#if LOG_ERROR_ENABLED
  #define Error(message, ...) _log_output(LogLevel_Error, message, ##__VA_ARGS__);
  #define ErrorInline(message, ...) _log_output_inline(LogLevel_Error, message, ##__VA_ARGS__)
  #define ErrorRaw(message, ...) _log_output_raw(LogLevel_Error, message, ##__VA_ARGS__)
#else
  #define Error(message, ...)
  #define ErrorInline(message, ...)
  #define ErrorRaw(message, ...)
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
