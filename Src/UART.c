#include "../Inc/UART.h"

int uart_fd;

int UART_Init(const char* UART_DEVICE, speed_t UART_BaudRate)
{
    /////
    uart_fd = open(UART_DEVICE, O_RDWR| O_NOCTTY | O_NDELAY);
    if (uart_fd == -1)
    {
        perror("Error opening UART");
        return 0;
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
    return 1;
}

void UART_write(const char *data, int size)
{
    int retries_UART_write = 0;
    do
    {
        int ret = write(uart_fd, data, size) ;
        if (ret == size)
            break;
        else if (ret == -1)
        {
            perror("Failed to write to UART");
            exit(EXIT_FAILURE);
        }
        else if (ret != size)
            printf("Error: Only %d out of %d bytes were written!\r\n", ret, size);

        retries_UART_write++;
        usleep(100);
    } while (retries_UART_write < MAX_RETRIES);

    if (retries_UART_write == MAX_RETRIES)
        fprintf(stderr, "Error: Maximum retries reached\r\n");
}

int UART_read(char *buffer, int size)
{
    int retries_UART_read = 0;
    do
    {
        int bytes_read = read(uart_fd, buffer, size);

        if (bytes_read == size)
            return 1;
        else if (bytes_read == 0)
        {
            printf("UART input buffer is empty.\n");
            break;
        }
        else
        {
            fprintf(stderr, "Error reading from UART: %s\n", strerror(errno));
            retries_UART_read++;
            usleep(100);
        }
    } while (retries_UART_read < MAX_RETRIES);

    if (retries_UART_read == MAX_RETRIES)
    {
        fprintf(stderr, "Error: Maximum retries reached\n");
        return 0;
    }
    return 0;
}

void UART_close()
{
    close(uart_fd);
}