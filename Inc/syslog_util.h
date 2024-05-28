#ifndef SYSLOG_UTIL_H
#define SYSLOG_UTIL_H

#include <stdarg.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define LOG_FACILITY LOG_LOCAL0

void syslog_init();
void syslog_log(int priority, const char *function_name, const char *message_type, const char *message, ...);
void syslog_close();

#endif