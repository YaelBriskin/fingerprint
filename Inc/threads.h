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
#include "GPIO.h"
#include "packet.h"
#include "UART.h"
#include "DataBase.h"
#include "FP_find_finger.h"
#include "lcd20x4_i2c.h"
#include "keypad.h"
#include "curl_client.h"
#include "tcp_server_socket.h"
#include "defines.h"
#include "config.h"

int getCurrent_UTC_Timestamp();
void buzzer();
void* fingerPrintThread(void* arg);
void* databaseThread(void* arg);
void *clockThread(void *arg);
void *resetDatabaseThread(void *arg);
void *handle_clientThread(void *arg);
void *socket_serverThread (void *arg);


#endif  // THREADS_H