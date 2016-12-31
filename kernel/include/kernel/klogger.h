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

void log(int16_t level, char* string, uint16_t length);
void logFErr(char* string, uint16_t length);
void logErro(char* string, uint16_t length);
void logWarn(char* string, uint16_t length);
void logNorm(char* string, uint16_t length);
void logDebg(char* string, uint16_t length);
void logFine(char* string, uint16_t length);

#endif
