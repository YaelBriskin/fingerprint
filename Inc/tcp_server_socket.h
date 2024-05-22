#ifndef TCP_SERVER_SOCKET_H
#define TCP_SERVER_SOCKET_H

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <syslog.h>
#include "../Inc/FP_delete.h"
#include "../Inc/DataBase.h"

#define SERVER_PORT 8080

int create_socket();
int read_data_from_client(int client_socket);

#endif 