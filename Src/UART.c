#include "../Inc/UART.h"

/**
 * @brief Initializes the UART interface.
 *
 * @param UART_DEVICE The UART device file.
 * @param UART_BaudRate The baud rate for the UART communication.
 * @return int The file descriptor for the UART device, or 0 on error.
 */
int UART_Init(const char* UART_DEVICE, speed_t UART_BaudRate)
{
    int uart_fd = open(UART_DEVICE, O_RDWR| O_NOCTTY );
    if (uart_fd == ERROR)
    {
        // Log the error if the UART device could not be opened
        syslog_log(LOG_ERR, __func__, "strerror", "Error opening UART", strerror(errno));
        return FAILED;
    }
    struct termios options;
    // Get the current configuration of the UART
    tcgetattr(uart_fd, &options);        
    // Set the baud rate for input and output
    cfsetispeed(&options, UART_BaudRate); 
    cfsetospeed(&options, UART_BaudRate);
    // Configure the UART for 8 data bits, no parity, 1 stop bit
    options.c_cflag = (options.c_cflag & ~CSIZE) | CS8;
    options.c_iflag = IGNBRK;
    options.c_lflag = 0;
    options.c_oflag = 0;
    options.c_cflag |= CLOCAL | CREAD;
    // Set VMIN and VTIME
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 5;
    // Disable software flow control
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    // Disable parity
    options.c_cflag &= ~(PARENB | PARODD);
    // Apply the settings to the UART immediately
    tcsetattr(uart_fd, TCSANOW, &options);
     // Flush the input (read) buffer
    tcflush(uart_fd, TCIOFLUSH);
    return uart_fd;
}

/**
 * @brief Writes data to the UART interface.
 *
 * @param uart_fd The file descriptor for the UART device.
 * @param data The data to write.
 * @param size The size of the data to write.
 */
void UART_write(int uart_fd,const char *data, int size)
{
    int retries_UART_write = 0;
    do
    {
        int ret = write(uart_fd, data, size) ;
        if (ret == size)
            break;
        else if (ret == ERROR)
        {
            // Log the error if the write operation fails
            syslog_log(LOG_ERR, __func__, "strerror", "Failed to write to UART", strerror(errno));
            exit(EXIT_FAILURE);
        }
        else if (ret != size)
        {
            // Log partial writes
            syslog_log(LOG_ERR, __func__, "format", "Error: Only %d out of %d bytes were written!", ret, size);
        }
        retries_UART_write++;
        usleep(DELAY);
    } while (retries_UART_write < g_max_retries);

    if (retries_UART_write == g_max_retries)
    {
        // Log the error if maximum retries are reached
        syslog_log(LOG_ERR, __func__, "strerr", "Error: Maximum retries reached", NULL);
    }
}

/**
 * @brief Reads data from the UART interface.
 *
 * @param uart_fd The file descriptor for the UART device.
 * @param buffer The buffer to store the read data.
 * @param size The size of the buffer.
 * @return int 1 if the read operation is successful, 0 otherwise.
 */
Status_t UART_read(int uart_fd,char *buffer, int size)
{
    int retries_UART_read = 0;
    do
    {
        int bytes_read = read(uart_fd, buffer, size);
        if (bytes_read == size)
        {
            return SUCCESS;
        }
        else if (bytes_read == 0)
        {
             // Log a message if the UART input buffer is empty
            syslog_log(LOG_ERR, __func__, "strerr", "UART input buffer is empty.", NULL);
            break;
        }
        else
        {
            // Log an error if reading from UART fails
            syslog_log(LOG_ERR, __func__, "strerror", "Error reading from UART:", strerror(errno));
            retries_UART_read++;
            usleep(DELAY);
        }
    } while (retries_UART_read < g_max_retries);

    if (retries_UART_read == g_max_retries)
    {
        // Log and print an error if maximum retries are reached
        syslog_log(LOG_ERR, __func__, "strerr", "Error: Maximum retries reached", NULL);
        return FAILED;
    }
    return FAILED;
}

/**
 * @brief Closes the UART interface.
 *
 * @param uart_fd The file descriptor for the UART device.
 */
void UART_close(int uart_fd)
{
    close(uart_fd);
}