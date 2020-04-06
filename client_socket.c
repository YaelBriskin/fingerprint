#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "192.168.1.70"
#define SERVER_PORT 60123

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char message[] = "Hello, server!";
    char buffer[1024];

    //create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created\n");
    //Setting up the sockaddr_in structure for the server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    printf("Setting up the sockaddr_in structure for the server was successful\n");
    // Connecting to the server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    printf("Socket connected to server\n");
    // Sending data
    while(1)
    {
        send(client_socket, message, strlen(message), 0);
        printf("Message sent to the server: %s\n", message);
        sleep(1);
    }
    // Closing a socket
    close(client_socket);

    return 0;
}
