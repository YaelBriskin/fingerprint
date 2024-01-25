#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "main.h"

#define SERVER_IP "192.168.1.70"
#define SERVER_PORT 60123

void connectToServer();
int sendData(const char* message);
void receiveData(char* buffer, size_t bufferSize);
void receiveDateTime(char* buffer, size_t bufferSize);
void closeConnection();
