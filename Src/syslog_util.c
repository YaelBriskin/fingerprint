#include "../Inc/syslog_util.h"

/**
 * @brief Initializes the syslog facility for logging.
 */
void syslog_init()
{
    extern const char *__progname;
    openlog(__progname, LOG_PID | LOG_CONS, LOG_FACILITY);
}

/**
 * @brief Logs a message to the syslog with a specified priority and message type.
 *
 * @param priority The priority level of the log message.
 * @param function_name The name of the function generating the log message.
 * @param message_type The type of the message (format, stderr, strerror).
 * @param message The message to log, which may include format specifiers.
 * @param ... Additional arguments for the format specifiers in the message.
 */
void syslog_log(int priority, const char *function_name, const char *message_type, const char *message, ...)
{
    va_list ap;
    va_start(ap, message);

    char log_message[MAX_LOG_MESSAGE_LENGTH];
    // Use a temporary buffer for formatting
    char formatted_message[MAX_LOG_MESSAGE_LENGTH];
    int message_len;

    if (message_type == NULL ||strcmp(message_type, "format") == 0)
    {
        // Format the message with variable arguments
        vsnprintf(formatted_message, MAX_LOG_MESSAGE_LENGTH, message, ap);
        // Create a log message with the prefix [function_name]
        message_len = snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "[%s] %s", function_name, formatted_message);
    }
    else if (strcmp(message_type, "stderr") == 0)
    {
        // Copy message from stderr
        message_len = snprintf(log_message,MAX_LOG_MESSAGE_LENGTH, "[%s] %s", function_name, message);
    }
    else if (strcmp(message_type, "strerror") == 0)
    {
        // Format message with error description from errno
        const char *error_desc = va_arg(ap, const char*);
        error_desc = error_desc ? error_desc : "(null)";
        message_len = snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "[%s] %s: %s", function_name, message, error_desc);
    }
    else if (strcmp(message_type, "OK") == 0)
    {
        // Format message with error description from errno
        char formatted_message[MAX_LOG_MESSAGE_LENGTH];
        // Format the message with variable arguments
        vsnprintf(formatted_message,MAX_LOG_MESSAGE_LENGTH, message, ap);        
        message_len = snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "[%s] SUCCESS: %s", function_name, formatted_message);
    }
    // Check for buffer overflow and log accordingly
    if (message_len >= 0 && message_len < MAX_LOG_MESSAGE_LENGTH)
    {
        syslog(priority, "%s", log_message); 
    }
    else
    {
        // Handle potential buffer overflow
        syslog(LOG_ERR, "[%s] Error: Message buffer overflow.", function_name);
    }
    va_end(ap);
}

/**
 * @brief Closes the syslog facility.
 */
void syslog_close()
{
    closelog();
}