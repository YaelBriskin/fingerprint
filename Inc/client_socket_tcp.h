#ifndef CLIENT_SOCKET_TCP_H
#define CLIENT_SOCKET_TCP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "packet.h"

#define SERVER_IP "192.168.1.70"
#define SERVER_PORT 60123
#define MAX_RETRIES 10

void connectToServer_tcp();
int socket_tcp_write(const char* message);
void socket_tcp_read(char* buffer, size_t bufferSize);
void close_tcp_Connection();

#endif  // CLIENT_SOCKET_TCP_H