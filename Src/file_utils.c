#include "../Inc/file_utils.h"

// Global variables for file pointers
FILE *file_URL = NULL;

pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Initializes the log file.
 *
 * This function opens a file for recording logs. If the file cannot be opened, an error is logged.
 *
 * @param file_name File name for logging.
 * @param file_ptr Pointer to the FILE into which the file will be written.
 */
void initFile(FILE **file, const char *file_name) 
{
    *file = fopen(file_name, "a"); // "a" means "append" - add to the end of the file
    if (file == NULL) 
    {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error opening file", strerror(errno));
    }
}

/**
 * @brief Writes the message to a file.
 *
 * This function writes the error message to a file.
 *
 * @param file Pointer to the file to write to.
 * @param func_name The name of the function where the error occurred.
 * @param message Error message.
 */
void writeToFile(FILE *file, const char *func_name, const char *message) 
{
    // Lock the mutex to ensure exclusive access to the file
    if (pthread_mutex_lock(&fileMutex) != 0)
    {
        syslog(LOG_ERR, "%s: Failed to lock mutex: %s", func_name, strerror(errno));
        return;
    }
    // Check if the file is open
    if (file != NULL) 
    {
        fprintf(file, "\nError in %s: %s", func_name, message);
    } else 
    {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "File not open", strerror(errno));

    }
    // Unlock the mutex to allow other threads to access the file
    if (pthread_mutex_unlock(&fileMutex) != 0) 
    {
        syslog(LOG_ERR, "Failed to unlock mutex in %s: %s", func_name, strerror(errno));
    }
}
/**
 * @brief Closes the file.
 *
 * This function closes the specified file.
 *
 * @param file Pointer to the file to close.
 */
void closeFile(FILE *file) 
{
    if (file != NULL) 
    {
        fclose(file);
        file = NULL;
    }
}