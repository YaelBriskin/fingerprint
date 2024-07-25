#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include "../Inc/syslog_util.h"
#include <stdio.h>


extern FILE *file_global;
extern FILE *file_URL;

void initFile(FILE **file, const char *file_name);
void writeToFile(FILE *file, const char *func_name, const char *message);
void closeFile(FILE *file);

#endif // FILE_UTILS_H