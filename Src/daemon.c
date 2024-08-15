#include "../Inc/daemon.h"


/**
 * @brief Sets up the signal handler for SIGTERM.
 */
void setup_signal_handler()
{
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGCHLD, &sa, NULL); // Ignore SIGCHLD

    sa.sa_handler = SIGTERM;
    sigaction(SIGTERM, &sa, NULL); // Handle SIGTERM
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