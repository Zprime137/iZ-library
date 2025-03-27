#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

// Log directory and file configuration
#define LOG_DIR "logs"               // Directory where logs are stored
#define LOG_FILE LOG_DIR "/log.txt"  // Default log file
#define LOG_MAX_SIZE 1024 * 1024 * 5 // Maximum log file size (5 MB)

// Log levels
typedef enum
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;

// Function declarations

/**
 * @brief Returns a string representation of the log level.
 *
 * @param level The log level.
 * @return A string corresponding to the log level.
 */
const char *log_level_to_string(LogLevel level);

/**
 * @brief Initializes the logging system.
 *
 * Creates the log directory if it doesn't exist and rotates logs if necessary.
 *
 * @param log_file The path to the log file.
 */
void log_init(const char *log_file);

/**
 * @brief Shuts down the logging system and cleans up resources.
 */
void log_shutdown(void);

/**
 * @brief Sets the current log level. Messages below this level will not be logged.
 *
 * @param level The log level to set.
 */
void log_set_log_level(LogLevel level);

/**
 * @brief Logs a formatted message at the given log level.
 *
 * This function logs messages to the log file in a thread-safe manner.
 *
 * @param level The log level for the message.
 * @param format The format string for the log message.
 */
void log_message(LogLevel level, const char *format, ...);

/**
 * @brief Logs a formatted message with extended information (file name, line number).
 *
 * This function logs messages to the log file in a thread-safe manner.
 *
 * @param level The log level for the message.
 * @param file_name The source file where the log was generated.
 * @param line_number The line number in the source file.
 * @param format The format string for the log message.
 */
void log_message_extended(LogLevel level, const char *file_name, int line_number, const char *format, ...);

/**
 * @brief Logs a formatted message to the console only, without requiring a log level.
 *
 * This can be used for debugging or tracking specific points in the code.
 *
 * @param format The format string for the log message.
 */
void log_console(const char *format, ...);

// Convenience logging functions for different log levels

/**
 * @brief Logs a debug message.
 *
 * @param format The format string for the log message.
 */
void log_debug(const char *format, ...);

/**
 * @brief Logs an info message.
 *
 * @param format The format string for the log message.
 */
void log_info(const char *format, ...);

/**
 * @brief Logs a warning message.
 *
 * @param format The format string for the log message.
 */
void log_warn(const char *format, ...);

/**
 * @brief Logs an error message.
 *
 * @param format The format string for the log message.
 */
void log_error(const char *format, ...);

/**
 * @brief Logs a fatal message.
 *
 * @param format The format string for the log message.
 */
void log_fatal(const char *format, ...);

#endif // LOGGER_H
