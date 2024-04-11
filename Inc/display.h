#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

#include "../Inc/lcd20x4_i2c.h"

extern bool showMessage;

void showMessageOnDisplay(const char *message, ...);

#endif /* DISPLAY_H */