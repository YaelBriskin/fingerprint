#include "../Inc/client_socket_http.h"

int client_socket;

int connectToServer() 
{
    struct sockaddr_in server_addr;
    int attempt = 0;
    struct hostent *server;

    // Create a socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr,"Socket creation failed");
       return -1;
    }
    printf("Socket created\n");

    // Obtain information about the host by its name    
    if ((server = gethostbyname(SERVER_HOST)) == NULL)
    {
        fprintf(stderr,"Invalid hostname");
        close(client_socket);
        return -1;
    }
    // Fill in the sockaddr_in structure for the server
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);

    printf("Setting up the sockaddr_in structure for the server was successful\n");
    // Attempt to connect MAX_RETRIES times
    while (attempt < MAX_RETRIES) 
    {
        // Connect to the server
        if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0) 
        {
            // Successful connection
            printf("Socket connected to server\n");
            return 0;
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
    fprintf(stderr, "Failed to connect after %d attempts\n", MAX_RETRIES);
    return -1;
}
int socket_write(const char* http_request)
{
    int attempt = 0;
    ssize_t bytesSent;
    while (attempt < MAX_RETRIES) 
    {
        // Send HTTP request
        bytesSent = send(client_socket, http_request, strlen(http_request), 0);
        if (bytesSent == -1) 
        {
            fprintf(stderr, "Error sending data");
            attempt++;
            printf("Retrying (%d/%d)...\n", attempt, MAX_RETRIES);
            sleep(1); 
        } 
        else 
        {
            printf("HTTP request  sent to the server\n");
            return 1;
        }
        fprintf(stderr, "Failed to send data after %d attempts\n", MAX_RETRIES);
        return 0;  
    }
}
int socket_read(char* buffer, size_t bufferSize)
{
    int attempt = 0;
    ssize_t bytesRead;
    while (attempt < MAX_RETRIES) 
    {
        //read data
        bytesRead = recv(client_socket, buffer, bufferSize - 1, 0);
        if (bytesRead > 0) 
        {
            // Successfully received data
            buffer[bytesRead] = '\0';
            printf("Received data from the server: %s\n", buffer);
            //GetFromServer(buffer);
            return 1;
        } 
        else if (bytesRead == 0) 
        {
            // Server closed connection
            printf("Server closed the connection\n");
            closeConnection();           
            connectToServer();
        } 
        else 
        {
            // Error while receiving data
            fprintf(stderr, "Error receiving data");

            if (attempt < MAX_RETRIES - 1) 
                sleep(1);
        }
        attempt++;
    }

        fprintf(stderr, "Failed to receive data after %d attempts\n", MAX_RETRIES);
        closeConnection();          
        connectToServer();
    return 0;
}
void closeConnection()
{
    // Close the connection
    close(client_socket);
    printf("Socket connection closed\n");
}
