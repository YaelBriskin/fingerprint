#include "client_socket.h"

int connectToServer() 
{
    int client_socket;
    struct sockaddr_in server_addr;
    int attempt = 0;

    // Create a socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created\n");

    // Fill in the sockaddr_in structure for the server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    printf("Setting up the sockaddr_in structure for the server was successful\n");
    // Attempt to connect MAX_RETRIES times
    while (attempt < MAX_RETRIES) 
    {
        // Connect to the server
        if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0) 
        {
            // Successful connection
            printf("Socket connected to server\n");
            break;
        }
        else 
        {
            // Connection error
            perror("Error connecting to server");
            close(client_socket);
            if (attempt < MAX_RETRIES - 1)
                sleep(1);
        }
        attempt++;
    }
    if (attempt == MAX_RETRIES) 
    {
        fprintf(stderr, "Failed to connect after %d attempts\n", MAX_RETRIES);
        exit(EXIT_FAILURE);
    }

    return client_socket;
}
int sendData(int socket, const char* message)
{
    int attempt = 0;
    ssize_t bytesSent;
    while (attempt < MAX_RETRIES) 
    {
        // Send data
        bytesSent = send(socket, message, strlen(message), 0);
        if (bytesSent == -1) 
        {
            perror("Error sending data");
            attempt++;
            printf("Retrying (%d/%d)...\n", attempt, MAX_RETRIES);
            sleep(1); 
        } 
        else 
        {
            printf("Message sent to the server: %s\n", message);
            return 1;
        }
        fprintf(stderr, "Failed to send data after %d attempts\n", MAX_RETRIES);
        return 0;  
    }
}
void receiveData(int socket, char* buffer, size_t bufferSize) {
    int attempt = 0;

    while (attempt < MAX_RECEIVE_RETRIES) {
        // Получение данных
        ssize_t bytesRead = recv(socket, buffer, bufferSize - 1, 0);

        if (bytesRead > 0) 
        {
            // Successfully received data
            buffer[bytesRead] = '\0';
            printf("Received data from the server: %s\n", buffer);
            break;  
        } 
        else if (bytesRead == 0) 
        {
            // Server closed connection
            printf("Server closed the connection\n");
            close(socket);
            exit(EXIT_FAILURE);
        } 
        else 
        {
            // Error while receiving data
            perror("Error receiving data");

            if (attempt < MAX_RECEIVE_RETRIES - 1) 
                sleep(1);
        }
        attempt++;
    }
    if (attempt == MAX_RECEIVE_RETRIES) 
    {
        fprintf(stderr, "Failed to receive data after %d attempts\n", MAX_RECEIVE_RETRIES);
        close(socket);
        exit(EXIT_FAILURE);
    }
}

void closeConnection(int socket)
{
    // Close the connection
    close(socket);
    printf("Socket connection closed\n");
}
