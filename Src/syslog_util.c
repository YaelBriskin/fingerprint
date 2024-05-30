#include "../Inc/syslog_util.h"

/**
 * @brief Initializes the syslog facility for logging.
 */
void syslog_init()
{
    extern const char *__progname;
    printf("syslog_init()  %s\n",__progname);
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

    char log_message[1000];
    int message_len;

    if (message_type == NULL ||strcmp(message_type, "format") == 0)
    {
        // Use a temporary buffer for formatting
        char formatted_message[1000];
        // Format the message with variable arguments
        vsnprintf(formatted_message, sizeof(formatted_message), message, ap);
        // Create a log message with the prefix [function_name]
        message_len = snprintf(log_message, sizeof(log_message), "[%s] %s", function_name, formatted_message);
    }
    else if (strcmp(message_type, "stderr") == 0)
    {
        // Copy message from stderr
        message_len = snprintf(log_message, sizeof(log_message), "[%s] %s", function_name, message);
    }
    else if (strcmp(message_type, "strerror") == 0)
    {
        // Format message with error description from errno
        const char *error_desc = va_arg(ap, const char*);
        message_len = snprintf(log_message, sizeof(log_message), "[%s] %s: %s", function_name, message, error_desc);
    }
    else if (strcmp(message_type, "OK") == 0)
    {
        // Format message with error description from errno
        char formatted_message[1000];
        // Format the message with variable arguments
        vsnprintf(formatted_message, sizeof(formatted_message), message, ap);        
        message_len = snprintf(log_message, sizeof(log_message), "[%s] SUCCESS: %s", function_name, formatted_message);
    }

    if (message_len >= 0 && message_len < sizeof(log_message))

        syslog(priority, "%s", log_message); 
    else
        // Handle potential buffer overflow
        syslog(LOG_ERR, "[%s] Error: Message buffer overflow.", function_name);

    va_end(ap);
}

/**
 * @brief Closes the syslog facility.
 */
void syslog_close()
{
    closelog();
}