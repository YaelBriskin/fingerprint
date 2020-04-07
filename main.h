#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "GPIO.h"
#include "UART.h"
#include "I2C.h"
#include "client_socket.h"
#include "DataBase.h"

#define MAX_RETRIES 10