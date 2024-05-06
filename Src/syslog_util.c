#include "../Inc/syslog_util.h"

#define LOG_FACILITY LOG_LOCAL0

void syslog_init()
{
    extern const char *__progname;
    printf("syslog_init()  %s\n",__progname);
    openlog(__progname, LOG_PID | LOG_CONS, LOG_FACILITY);
}

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
        // Копируем сообщение из stderr
        message_len = snprintf(log_message, sizeof(log_message), "[%s] %s", function_name, message);
    }
    else if (strcmp(message_type, "strerror") == 0)
    {
        // Форматируем сообщение с описанием ошибки errno
        const char *error_desc = va_arg(ap, const char*);
        message_len = snprintf(log_message, sizeof(log_message), "[%s] %s: %s", function_name, message, error_desc);
    }

    if (message_len >= 0 && message_len < sizeof(log_message))
    {
        // // Append stderr message if provided
        // if (stderr_message != NULL)
        // {
        //     strncat(log_message, "\n", sizeof(log_message) - message_len - 1);
        //     strncat(log_message, stderr_message, sizeof(log_message) - message_len - 2);
        // }

        // // Log the formatted message
        // vsyslog(priority, log_message);
        syslog(priority, "%s", log_message); 
    }
    else
        // Handle potential buffer overflow
        syslog(LOG_ERR, "[%s] Error: Message buffer overflow.", function_name);

    va_end(ap);
}

void syslog_close()
{
    closelog();
}