#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#define FOREACH_LOGLEVEL(DO, ...) \
  DO(DEBUG, 10, ##__VA_ARGS__)    \
  DO(INFO, 20, ##__VA_ARGS__)     \
  DO(WARN, 30, ##__VA_ARGS__)     \
  DO(ERROR, 40, ##__VA_ARGS__)

#define _DEFINE_LVL(LVL, VALUE) LOG_##LVL = VALUE,
typedef enum { FOREACH_LOGLEVEL(_DEFINE_LVL) } LogLevel;
#undef _DEFINE_LVL

static inline bool LogLevel_IsValid(LogLevel lvl);

static inline const char *LogLevel_ToString(LogLevel lvl);
bool LogLevel_FromString(const char *str, LogLevel *d_lvl);

LogLevel Logging_Level(LogLevel *new_lvl);

void vLog(LogLevel lvl, const char *fmt, va_list args);
static inline void Log(LogLevel lvl, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

#define DEBUG(FMT, ...) Log(LOG_DEBUG, FMT, ##__VA_ARGS__)
#define INFO(FMT, ...) Log(LOG_INFO, FMT, ##__VA_ARGS__)
#define WARN(FMT, ...) Log(LOG_WARN, FMT, ##__VA_ARGS__)
#define ERROR(FMT, ...) Log(LOG_ERROR, FMT, ##__VA_ARGS__)

/*****************************************************************************/
/*                             STATIC INLINE IMPL                            */
/*****************************************************************************/

#define _RETURN_STRING_LVL(LVL, ...) \
  case LOG_##LVL:                    \
    return #LVL;

static inline const char *LogLevel_ToString(LogLevel lvl) {
  switch (lvl) { FOREACH_LOGLEVEL(_RETURN_STRING_LVL) }
  return NULL;
}

#undef _RETURN_STRING_LVL

static inline bool LogLevel_IsValid(LogLevel lvl) {
  return LogLevel_ToString(lvl) != NULL;
}

static inline void Log(LogLevel lvl, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vLog(lvl, fmt, args);
  va_end(args);
}
