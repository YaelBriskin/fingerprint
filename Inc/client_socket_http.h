#ifndef CLIENT_SOCKET_HTTP_H
#define CLIENT_SOCKET_HTTP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "packet.h"
#include <errno.h>     
#include <netdb.h>

#define SERVER_HOST "example.com"
#define SERVER_PORT 80
#define MAX_RETRIES 10

int connectToServer();
int socket_write(const char* message);
int socket_read(char* buffer, size_t bufferSize);
void closeConnection();

#endif  // CLIENT_SOCKET_HTTP_H