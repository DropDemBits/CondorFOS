#include <stdint.h>

#ifndef _KLOGGER_H
#define _KLOGGER_H

/* Log levels */
#define LOG_FATAL   +3
#define LOG_ERROR   +2
#define LOG_WARNING +1
#define LOG_NORMAL  +0
#define LOG_INFO    -1
#define LOG_DEBUG   -2
#define LOG_FINE    -3

/**
 * Prints an event
 * @param level The level of the log
 * @param string The string to print
 */
void log(int16_t level, const char* string);

/**
 * Prints a fatal error
 * @param string The string to print
 */
void logFErr(const char* string);

/**
 * Prints an error
 * @param string The string to print
 */
void logErro(const char* string);

/**
 * Prints a warning
 * @param string The string to print
 */
void logWarn(const char* string);

/**
 * Prints Normal stuff
 * @param string The string to print
 */
void logNorm(const char* string);

/**
 * Prints some information
 * @param string The string to print
 */
void logInfo(const char* string);

/**
 * Prints debug information
 * @param string The string to print
 */
void logDebg(const char* string);

/**
 * Prints deep debugging information
 * @param string The string to print
 */
void logFine(const char* string);

#endif
