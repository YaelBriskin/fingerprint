#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdint.h>
#include "lcd16x2_i2c.h"
#include "defines.h"
#include <time.h>
#include "config.h"


#define GPIO_PIN_ROW1 45  //P8_11
#define GPIO_PIN_ROW2 44  //P8_12
#define GPIO_PIN_ROW3 26  //P8_14
#define GPIO_PIN_ROW4 47  //P8_15

#define GPIO_PIN_COL1 46  //P8_16
#define GPIO_PIN_COL2 27  //P8_17
#define GPIO_PIN_COL3 65  //P8_18

#define NUM_ROWS 4
#define NUM_COLS 3
#define TOTAL_PINS (NUM_ROWS + NUM_COLS)

Status_t keypad_init();
int enter_ID_keypad();
char read_keypad_value();
int check_timeout(struct timespec start_time, int timeout_seconds);
void beginDisplay();

#endif