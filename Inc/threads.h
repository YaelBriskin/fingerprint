#ifndef THREADS_H
#define THREADS_H

#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "../Inc/GPIO.h"
#include "../Inc/packet.h"
#include "../Inc/UART.h"
#include "../Inc/DataBase.h"
#include "../Inc/FP_find_finger.h"
#include "../Inc/lcd20x4_i2c.h"
#include "../Inc/keypad.h"
#include "../Inc/display.h"

#define MAX_RETRIES 10
#define SIZE_TZ 9

extern volatile bool isRunning;

void getCurrentTimeAndDate(char*  dateString , char* timeString);
void* fingerPrintThread(void* arg);
void* databaseThread(void* arg);
void* socketThread(void* arg);
void *displayThread(void *arg);
#endif  // THREADS_H