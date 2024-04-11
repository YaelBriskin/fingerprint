#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define GPIO_BUTTON_1 66  
#define GPIO_BUTTON_2 67 
#define GPIO_LED 68

void GPIO_init(int pinNumber, const char* direction);
int GPIO_read(int pinNumber);
int GPIO_open(int pinNumber);
void GPIO_close(int gpio_fd);

void turnOnLED();
