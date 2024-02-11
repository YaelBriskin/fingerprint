#ifndef FP_FIND_FINGER_H
#define FP_FIND_FINGER_H

#include <stdio.h>
#include <time.h>
#include "UART.h"
#include "lcd20x4_i2c.h"
#include "packet.h"


int findFinger(const char* message);
int stringToInt(const char* str);

#endif  // FP_FIND_FINGER_H
