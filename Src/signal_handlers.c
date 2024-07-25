#include "../Inc/signal_handlers.h"

// External declarations of condition variables
extern pthread_cond_t displayCond;
extern pthread_cond_t databaseCond;
// External declarations of mutexes
extern pthread_mutex_t displayMutex;
extern pthread_mutex_t databaseMutex;
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
    syslog_log(LOG_INFO, __func__, "stderr", "SIGINT signal received");
    
    // Terminate threads
    stop = 1;

    // Clean up resources
    syslog_close();
    curl_global_cleanup();
    DB_close();
    UART_close(uart2_fd);
    UART_close(uart4_fd);
    I2C_close();
    closeFile(file_global);
    closeFile(file_URL);
    //Destroying the condition variable and mutex
    pthread_cond_destroy(&databaseCond);
    pthread_cond_destroy(&displayCond);
    pthread_mutex_destroy(&sqlMutex);
    pthread_mutex_destroy(&databaseMutex);
    pthread_mutex_destroy(&displayMutex);
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
        syslog_log(LOG_ERR, __func__, "strerror", "Error setting up sigaction", strerror(errno));
        exit(EXIT_FAILURE);
    }
}