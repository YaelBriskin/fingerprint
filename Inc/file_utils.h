#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include "../Inc/syslog_util.h"
#include <stdio.h>

void initFile();
void writeToFile(const char *func_name, const char *error_message);
void closeFile();

#endif // FILE_UTILS_H