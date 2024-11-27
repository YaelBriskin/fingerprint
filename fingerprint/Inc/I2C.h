#ifndef I2C_H
#define I2C_H

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
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "syslog_util.h"
#include "defines.h"
#include "config.h"

#define I2C_BUS "/dev/i2c-1" 
#define I2C_ADDRESS 0x27 
// I2C_SDA  GPIO 2  pin 3
// I2C_SCL  GPIO 3  pin 5

Status_t I2C_Init();
void I2C_write(uint8_t *buffer, int size);
void I2C_close();

#endif /* I2C_H */
