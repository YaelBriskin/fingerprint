#include "../Inc/tcp_server_socket.h"

/**
 * @brief Creates a server socket.
 *
 * @return int The server socket file descriptor on success, -1 on failure.
 */
int create_server_socket()
{
    int server_socket;
    struct sockaddr_in server_address;
    int optval = 1;
    // Create a server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == ERROR)
    {
        syslog_log(LOG_ERR, __func__, "strerror", "Error create a socket", strerror(errno));
        return ERROR;
    }
    // Настройка SO_REUSEADDR
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == ERROR)
    {
        syslog_log(LOG_ERR, __func__, "strerror", "setsockopt (SO_REUSEADDR)", strerror(errno));
        return ERROR;
    }
    // Настройка SO_KEEPALIVE
    if (setsockopt(server_socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) == ERROR)
    {
        syslog_log(LOG_ERR, __func__, "strerror", "setsockopt (SO_KEEPALIVE)", strerror(errno));
        return ERROR;
    }

    // Setting up the server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(g_server_port);

    // Binding a socket to an address
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == ERROR)
    {
        // Log error if binding fails
        syslog_log(LOG_ERR, __func__, "strerror", "Error binding a socket", strerror(errno));
        return ERROR;
    }

    // Listen on socket
    if (listen(server_socket, 5) == ERROR)
    {
        // Log error if listening fails
        syslog_log(LOG_ERR, __func__, "strerror", "Error listen on socket", strerror(errno));
        return ERROR;
    }

    return server_socket;
}
/**
 * @brief Reads data from a client socket.
 *
 * @param client_socket The client socket file descriptor.
 * @param client_id A pointer to an integer where the client ID will be stored.
 * @return int 0 on success, 1 on failure.
 */
Status_t read_data_from_client(int client_socket, int *client_id) 
{
    int received_bytes;
    // Receive the client ID from the client
    received_bytes = recv(client_socket, client_id, sizeof(int), 0);
    if (received_bytes == ERROR)
    {
        // Log error if receiving data fails
        syslog_log(LOG_ERR, __func__, "strerror", "Error receive from client", strerror(errno));
        return FAILED;
    }

    // Checking the correctness of the received data
    if (received_bytes != sizeof(int))
    {
        // Log error if the received data format is incorrect
        syslog_log(LOG_ERR, __func__, "strerror", "Error Invalid data format from client", strerror(errno));
        return FAILED;
    }
    printf("Received client ID for deletion: %d\n", client_id);
    return SUCCESS;
}