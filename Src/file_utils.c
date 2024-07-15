#include "../Inc/file_utils.h"

// Global variable to store a pointer to a file
FILE *file = NULL;
// Function to initialize the file
void initFile(const char *file_name) 
{
    file = fopen(file_name, "a"); // "a" means "append" - add to the end of the file
    if (file == NULL) 
    {
        syslog_log(LOG_ERR, __func__, "strerror", "Error opening file", strerror(errno));
    }
}
void writeToFile(const char *func_name, const char *error_message) 
{
    if (file != NULL) 
    {
        fprintf(file, "Error in %s: %s\n", func_name, error_message);
    } else 
    {
        syslog_log(LOG_ERR, __func__, "strerror", "File not open", strerror(errno));

    }
}
// Function to close the file
void closeFile() 
{
    if (file != NULL) 
    {
        fclose(file);
        file = NULL;
    }
}