#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define GPIO_BUTTON_IN 66  
#define GPIO_BUTTON_OUT 67 
#define GPIO_BUTTON_NEW 68

int GPIO_init(int pinNumber, const char* direction);
int GPIO_read(int pinNumber);
int GPIO_open(int pinNumber);
void GPIO_close(int gpio_fd);

void turnOnLED();
