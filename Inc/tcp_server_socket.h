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
#include "FP_delete.h"
#include "DataBase.h"
#include "defines.h"
#include "config.h"

#define SERVER_PORT 64789

int create_server_socket();
Status_t read_data_from_client(int client_socket, int *client_id);

#endif 