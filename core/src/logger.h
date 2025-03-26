#pragma once

#include "defines.h"

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

b8 initialize_logging(u64* memory_requirement, void* state);
void shutdown_logging();

KAPI void log_output(LogLevel level, const char* message, ...);

// Logs a fatal-level message.
#define Fatal(message, ...) log_output(LOG_LEVEL_FATAL, message, ##__VA_ARGS__)

#ifndef KERROR
// Logs an error-level message.
#define Error(message, ...) log_output(LOG_LEVEL_ERROR, message, ##__VA_ARGS__)
#endif

#if LOG_WARN_ENABLED == 1
// Logs a warning-level message.
#define Warn(message, ...) log_output(LOG_LEVEL_WARN, message, ##__VA_ARGS__)
#else
// Does nothing when LOG_WARN_ENABLED != 1
#define Warn(message, ...);
#endif

#if LOG_INFO_ENABLED == 1
// Logs a info-level message.
#define Info(message, ...) log_output(LOG_LEVEL_INFO, message, ##__VA_ARGS__)
#else
// Does nothing when LOG_INFO_ENABLED != 1
#define Info(message, ...);
#endif

#if LOG_DEBUG_ENABLED == 1
// Logs a debug-level message.
#define Debug(message, ...) log_output(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__)
#else
// Does nothing when LOG_DEBUG_ENABLED != 1
#define Debug(message, ...);
#endif

#if LOG_TRACE_ENABLED == 1
// Logs a trace-level message.
#define Trace(message, ...) log_output(LOG_LEVEL_TRACE, message, ##__VA_ARGS__)
#else
// Does nothing when LOG_TRACE_ENABLED != 1
#define Trace(message, ...);
#endif

