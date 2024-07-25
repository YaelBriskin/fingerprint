#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdint.h>
#include "../Inc/UART.h"
#include "../Inc/lcd20x4_i2c.h"
#include "defines.h"
#include <time.h>
#include "config.h"

int enter_ID_keypad();
char convert_to_char(uint8_t value);

#endif