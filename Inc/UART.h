#ifndef UART_H
#define UART_H

#include <unistd.h> 
#include <sys/types.h> 
#include <errno.h>     
#include <stdio.h>     
#include <sys/wait.h>  
#include <stdlib.h>  
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include "syslog_util.h"

#define UART4_DEVICE "/dev/ttyS4"
#define UART2_DEVICE "/dev/ttyS2"
#define UART2_BaudRate B57600
#define UART4_BaudRate B9600

#define MAX_RETRIES 10


int UART_Init(const char* device, speed_t UART_BaudRate);
void UART_write(int uart_fd,const char* data, int size);
int UART_read(int uart_fd,char* buffer, int size);
void UART_close(int uart_fd)
;

#endif 