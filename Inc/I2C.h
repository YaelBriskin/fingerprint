#include <unistd.h>   
#include <sys/types.h> 
#include <errno.h>     
#include <stdio.h>     
#include <sys/wait.h>  
#include <stdlib.h>  
#include <fcntl.h>
#include <string.h>
#include <unistd.h>   
#include <stdint.h>
#include "main.h"

#define I2C_BUS "/dev/i2c-2" 

void I2C_Init();
void I2C_write(uint8_t *buffer, int size);
void I2C_close();
