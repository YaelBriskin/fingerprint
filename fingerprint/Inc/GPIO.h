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

#define GPIO_LED_RED 69
#define GPIO_BUZZER 68

#define GPIO_PIN_ROW1 45  //P8_11
#define GPIO_PIN_ROW2 44  //P8_12
#define GPIO_PIN_ROW3 26  //P8_14
#define GPIO_PIN_ROW4 47  //P8_15

#define GPIO_PIN_COL1 46  //P8_16
#define GPIO_PIN_COL2 27  //P8_17
#define GPIO_PIN_COL3 65  //P8_18

Status_t GPIO_init(int pinNumber, const char* direction);
int GPIO_read(int pinNumber);
void GPIO_write(int gpio_fd, int value);
int GPIO_open(int pinNumber, int flag);
void GPIO_close(int gpio_fd);

void turnOnLED();

#endif 