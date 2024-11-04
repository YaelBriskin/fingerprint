#ifndef GPIO_H
#define GPIO_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "syslog_util.h"
#include "defines.h"
#include "config.h"

#define GPIO_BUTTON_IN 66  
#define GPIO_BUTTON_OUT 67 
#define GPIO_BUTTON_NEW 68

#define GPIO_LED_RED 69
#define GPIO_BUZZER 44

Status_t GPIO_init(int pinNumber, const char* direction);
int GPIO_read(int pinNumber);
void GPIO_write(int gpio_fd, int value);
int GPIO_open(int pinNumber, int flag);
void GPIO_close(int gpio_fd);

void turnOnLED();

#endif 