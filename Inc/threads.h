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
#include "../Inc/client_socket.h"
#include "../Inc/UART.h"
#include "../Inc/DataBase.h"
#include "../Inc/FP_find_finger.h"

#define MAX_RETRIES 10

void getCurrentTimeAndDate(char*  dateString , char* timeString);
void* fingerPrintThread(void* arg);
void* databaseThread(void* arg);
void* socketThread(void* arg);

#endif  // THREADS_H