#include "UART.h"

int uart_fd;

void UART_Init()
{
    uart_fd = open(UART_DEVICE, O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_fd == -1)
    {
        perror("Error opening UART");
        exit(EXIT_FAILURE);
    }
    struct termios options;
    /* Serial Configuration */
    tcgetattr(uart_fd, &options);         // Get Current Config
    cfsetispeed(&options, UART_BaudRate); // Set Baud Rate
    cfsetospeed(&options, UART_BaudRate);
    options.c_cflag = (options.c_cflag & ~CSIZE) | CS8;
    options.c_iflag = IGNBRK;
    options.c_lflag = 0;
    options.c_oflag = 0;
    options.c_cflag |= CLOCAL | CREAD;
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 5;
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_cflag &= ~(PARENB | PARODD);
    /* Save The Configure */
    tcsetattr(uart_fd, TCSANOW, &options);
    /* Flush the input (read) buffer */
    tcflush(uart_fd, TCIOFLUSH);
}

void UART_write(const char *data, int size)
{
    int retries_UART_write = 0;
    do
    {
        if (write(uart_fd, data, size) == size)
            break;
        perror("Failed to write to UART");
        retries_UART_write++;
    } while (retries_UART_write < MAX_RETRIES);

    if (retries_UART_write == MAX_RETRIES)
        fprintf(stderr, "Error: Maximum retries reached\n");
}

int UART_read(char *buffer, int size)
{
    int retries_UART_read = 0;
    do
    {
        if (read(uart_fd, buffer, size) == size)
            return 1;
        perror("Error reading from UART");
        retries_UART_read++;
    }while (retries_UART_read < MAX_RETRIES);

    if (retries_UART_read == MAX_RETRIES)
        fprintf(stderr, "Error: Maximum retries reached\n");
    return 0;
}

void UART_close()
{
    close(uart_fd);
}