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
#include "../Inc/curl_client.h"
#include "../Inc/tcp_server_socket.h"

#define MAX_RETRIES 10

int getCurrent_UTC_Timestamp();
void buzzer();
void* fingerPrintThread(void* arg);
void* databaseThread(void* arg);
void *clockThread(void *arg);
void *resetDatabaseThread(void *arg);
void *handle_clientThread(void *arg);
void *socket_serverThread (void *arg);


#endif  // THREADS_H