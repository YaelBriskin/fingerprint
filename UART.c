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
}
int Uart_Config()
{
  struct termios options;
  /* Serial Configuration */
  tcgetattr(uart_fd, &options);   // Get Current Config
  cfsetispeed(&options, UART_BaudRate); // Set Baud Rate
  cfsetospeed(&options, UART_BaudRate);
  options.c_cflag = (options.c_cflag & ~CSIZE) | CS8;
  options.c_iflag =  IGNBRK;
  options.c_lflag = 0;
  options.c_oflag = 0;
  options.c_cflag |= CLOCAL | CREAD;
  options.c_cc[VMIN] = 1;
  options.c_cc[VTIME] = 5;
  options.c_iflag &= ~(IXON|IXOFF|IXANY);
  options.c_cflag &= ~(PARENB | PARODD);
  /* Save The Configure */
  tcsetattr(uart_fd, TCSANOW, &options);
  /* Flush the input (read) buffer */
  tcflush(uart_fd,TCIOFLUSH);
}

void UART_transmit(const char* data, int size) 
{
    int retries_UART_transmit = 0;
    do {
        if (write(uart_fd, data, size) == size)
            break;
        perror("Failed to write to I2C bus");
        retries_UART_transmit++;
    } while (retries_UART_transmit < MAX_RETRIES);

    if (retries_UART_transmit == MAX_RETRIES) 
        fprintf(stderr, "Error: Maximum retries reached\n");
}
void UART_close()
{
    close(uart_fd);
}