#include "../Inc/daemon.h"

void signal_handler(int signo)
{
    switch (signo)
    {
        case SIGTERM:
            syslog_log(LOG_ERR, __func__, "stderr", "Received SIGTERM signal. Terminating...");
            exit(EXIT_SUCCESS); 
            break;
        case SIGCHLD:
            syslog_log(LOG_ERR, __func__, "stderr", "Received SIGCHLD signal.");
            break;

        default:
            syslog_log(LOG_ERR, __func__, "stderr", "Received unknown signal.");
            break;
    }
}
/**
 * @brief Sets up the signal handler for SIGTERM.
 */
void setup_signal_handler()
{
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGTERM, &sa, NULL) == -1) 
    {
        char log_message[MAX_LOG_MESSAGE_LENGTH];
        snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Failed to set up SIGTERM handler. Error: %s", strerror(errno));
        syslog_log(LOG_ERR, __func__, "stderr",log_message);
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGCHLD, &sa, NULL) == -1) 
    {
        char log_message[MAX_LOG_MESSAGE_LENGTH];
        snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Failed to set up SIGCHLD handler. Error: %s", strerror(errno));
        syslog_log(LOG_ERR, __func__, "stderr",log_message);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Initializes the daemon process.
 *
 * This function performs the steps necessary to turn the current process into a daemon:
 * 1. Fork the process and exit the parent.
 * 2. Create a new session.
 * 3. Set the file permissions mask.
 * 4. Change the working directory to root.
 * 5. Close standard input, output, and error file descriptors.
 * 
 * @return void
 */
void init_daemon()
{
    pid_t pid;
    // Fork the process
    pid = fork();
    // If the fork failed, exit
    if (pid < 0)
        exit(EXIT_FAILURE);
    // If we got a good PID, then exit the parent process
    if (pid > 0)
        exit(EXIT_SUCCESS);
    // Create a new session
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    // Fork again to prevent the process from acquiring a controlling terminal
    pid = fork();
    // If the fork failed, exit
    if (pid < 0)
        exit(EXIT_FAILURE);
    // If we got a good PID, then exit the parent process.
    if (pid > 0)
        exit(EXIT_SUCCESS);

    // Set file permissions mask to 0
    umask(0);

    // Change the working directory to the root directory
    if (chdir("/") < 0)
        exit(EXIT_FAILURE);

    // Close all open file descriptors
    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--)
    {
        close(x);
    }
}