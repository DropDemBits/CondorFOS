#include <stdint.h>

#ifndef KLOGGER_H
#define KLOGGER_H

/* Log levels */
#define LOG_FATAL   +3
#define LOG_ERROR   +2
#define LOG_WARNING +1
#define LOG_NORMAL  +0
#define LOG_DEBUG   -1
#define LOG_FINE    -2

void log(int16_t level, const char* string);
void logFErr(const char* string);
void logErro(const char* string);
void logWarn(const char* string);
void logNorm(const char* string);
void logDebg(const char* string);
void logFine(const char* string);

#endif
