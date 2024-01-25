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

void UART_Init();
void Uart_Config();
void UART_write(const char* data, int size);
int UART_read(char* buffer, int size);
void UART_close();
