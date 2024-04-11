#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdint.h>
#include "../Inc/UART.h"
#include "../Inc/lcd20x4_i2c.h"
#include "../Inc/display.h"

#define MAX_RETRIES 10
#define MAX_LENGTH_ID 3
#define MAX_FINGERPRINT 100

int receive_ID_keypad();
char convert_to_char(uint8_t value);