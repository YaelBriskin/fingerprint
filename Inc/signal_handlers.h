#ifndef SIGNAL_HANDLERS_H
#define SIGNAL_HANDLERS_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include "DataBase.h"
#include "curl_client.h"
#include "I2C.h"
#include "UART.h"
#include "syslog_util.h"
#include "file_utils.h"

void handle_sigint(int sig);
void setup_sigint_handler();

#endif // SIGNAL_HANDLERS_H
