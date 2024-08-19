#ifndef DAEMON_H
#define DAEMON_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <string.h>
#include "file_utils.h"

void signal_handler(int signo);
void setup_signal_handler();
void init_daemon();

#endif