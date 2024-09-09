#include "../Inc/signal_handlers.h"

extern pthread_t thread_datetime;
extern pthread_t thread_database;
extern pthread_t thread_deletion;
// External declarations of condition variables
extern pthread_cond_t displayCond;
extern pthread_cond_t databaseCond;
extern pthread_cond_t requestCond;

// External declarations of mutexes
extern pthread_mutex_t displayMutex;
extern pthread_mutex_t databaseMutex;
extern pthread_mutex_t requestMutex;

extern pthread_mutex_t sqlMutex;

extern volatile sig_atomic_t stop;
extern int uart2_fd, uart4_fd;
// External declarations of file
extern FILE *file_global;
extern FILE *file_URL;

// Signal handler for SIGINT
/**
 * @brief Handles the SIGINT signal (Ctrl+C).
 *
 * This function is called when the SIGINT signal is received. It logs the
 * reception of the signal, sets the stop flag, and performs cleanup of various
 * resources including closing files, closing database connections, and destroying
 * condition variables and mutexes.
 *
 * @param sig The signal number.
 */
void handle_sigint(int sig)
{
    int retval;
    LOG_MESSAGE(LOG_INFO, __func__, "stderr", "SIGINT signal received", NULL);

    // Set the stop flag
    stop = 1;

    // Signal the condition variables to wake up waiting threads
    pthread_mutex_lock(&displayMutex);
    if (pthread_cond_signal(&displayCond) != 0)
    {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error signaling displayCond", strerror(errno));
    }
    pthread_mutex_unlock(&displayMutex);

    pthread_mutex_lock(&databaseMutex);
    if (pthread_cond_signal(&databaseCond) != 0)
    {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error signaling databaseCond", strerror(errno));
    }
    pthread_mutex_unlock(&databaseMutex);

    pthread_mutex_lock(&requestMutex);
    if (pthread_cond_signal(&requestCond) != 0)
    {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error signaling requestCond", strerror(errno));
    }
    pthread_mutex_unlock(&requestMutex);

    // Wait for threads to finish
    retval = pthread_join(thread_datetime, NULL);
    if (retval != 0)
    {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error joining thread_datetime", strerror(retval));
    }
    retval = pthread_join(thread_database, NULL);
    if (retval != 0)
    {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error joining thread_database", strerror(retval));
    }
    retval = pthread_join(thread_deletion, NULL);
    if (retval != 0)
    {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error joining thread_deletion", strerror(retval));
    }

    // Destroy condition variables and mutexes with error checking
    if (pthread_cond_destroy(&databaseCond) != 0)
    {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error destroying databaseCond", strerror(errno));
    }
    if (pthread_cond_destroy(&displayCond) != 0)
    {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error destroying displayCond", strerror(errno));
    }
    if (pthread_mutex_destroy(&sqlMutex) != 0)
    {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error destroying sqlMutex", strerror(errno));
    }
    if (pthread_mutex_destroy(&databaseMutex) != 0)
    {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error destroying databaseMutex", strerror(errno));
    }
    if (pthread_mutex_destroy(&displayMutex) != 0)
    {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error destroying displayMutex", strerror(errno));
    }
    // Clean up resources
    curl_global_cleanup();
    DB_close();
    UART_close(uart2_fd);
    UART_close(uart4_fd);
    I2C_close();
    closeFile(file_URL);

    // Close syslog last
    syslog_close();
    // Exit the program
    exit(0);
}

// Function to set the SIGINT signal handler
/**
 * @brief Sets up the SIGINT signal handler.
 *
 * This function sets up the signal handler for the SIGINT signal (Ctrl+C).
 * It uses the sigaction system call to register the handle_sigint function
 * as the handler for SIGINT.
 */
void setup_sigint_handler()
{
    // Define a sigaction structure to specify the signal handler
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    // Set up the SIGINT signal handler
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error setting up sigaction", strerror(errno));
        exit(EXIT_FAILURE);
    }
}