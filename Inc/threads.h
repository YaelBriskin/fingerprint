#ifndef THREADS_H
#define THREADS_H

#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "defines.h"
#include "GPIO.h"
#include "packet.h"
#include "UART.h"
#include "DataBase.h"
#include "FP_find_finger.h"
#include "lcd16x2_i2c.h"
#include "keypad.h"
#include "curl_client.h"
#include "config.h"

//---functions
int getCurrent_UTC_Timestamp();
void buzzer();
//---threads
void* databaseThread(void* arg);
void *clockThread(void *arg);
void *post_requestThread(void *arg);

#endif  // THREADS_H