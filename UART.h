#include <unistd.h>   
#include <sys/types.h> 
#include <errno.h>     
#include <stdio.h>     
#include <sys/wait.h>  
#include <stdlib.h>  
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include "main.h"

#define UART_DEVICE "/dev/ttyS1"
#define UART_BaudRate B9600

int UART_Init();
void Uart_Config(int uart_fd);
void UART_write(int uart_fd,const char* data, int size);
int UART_read(int uart_fd,char* buffer, int size);
void UART_close(int uart_fd);
