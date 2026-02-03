#include "logging.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define _IS_EQUAL_TO(LVL, _, STR, DEST)                 \
  if (strcmp(STR, LogLevel_ToString(LOG_##LVL)) == 0) { \
    DEST = LOG_##LVL;                                   \
    return true;                                        \
  } else

bool LogLevel_FromString(const char *str, LogLevel *d_lvl) {
  assert(str != NULL);
  assert(d_lvl != NULL);
  FOREACH_LOGLEVEL(_IS_EQUAL_TO, str, *d_lvl) { return false; }
}

#undef _IS_EQUAL_TO

LogLevel Logging_Level(LogLevel *new_lvl) {
  static LogLevel m_current_lvl = LOG_WARN;

  if ((new_lvl != NULL) && LogLevel_IsValid(*new_lvl)) {
    m_current_lvl = *new_lvl;
  }

  return m_current_lvl;
}

void vLog(LogLevel lvl, const char *fmt, va_list args) {
  if (lvl >= Logging_Level(NULL)) {
    FILE *const f = stderr;

    struct timespec now;
    if (clock_gettime(CLOCK_REALTIME, &now) == 0) {
      static char stamp[(4 + 2 + 2) + 2 // 'YYYY-MM-DD' <- strftime (%F)
                        + 1             // ' '          <- Separator
                        + (2 * 3) + 2   // 'HH:MM:SS'   <- strftime (%T)
                        + 1             // ','          <- Separator
                        + 3             // 'XXX'        <- msec
                        + 1             // NULL TERMINATED
      ] = {'\0'};

      size_t cnt =
          strftime(stamp, sizeof(stamp), "%F %T", localtime(&now.tv_sec));

      snprintf(stamp + cnt, sizeof(stamp) - cnt, ",%.3li",
               (now.tv_nsec / (unsigned)(1e6)));

      fprintf(f, "[%.*s]", (int)sizeof(stamp), stamp);
    }

    fprintf(f, "[fmap][%-5s]: ", LogLevel_ToString(lvl));
    vfprintf(f, fmt, args);
    fputc('\n', f);
    fflush(f);
  }
}
