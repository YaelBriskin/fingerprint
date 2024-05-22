#include "../Inc/tcp_server_socket.h"

int create_socket()
{
    int server_socket;
    struct sockaddr_in server_address;
    int optval = 1;
    // Create a server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        syslog_log(LOG_ERR, __func__, "strerror", "Error create a socket", strerror(errno));
        return -1;
    }

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    {
        syslog_log(LOG_ERR, __func__, "strerror", "setsockopt (SO_REUSEADDR)", strerror(errno));
        return -1;
    }

    if (setsockopt(server_socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) == -1)
    {
        syslog_log(LOG_ERR, __func__, "strerror", "setsockopt (SO_KEEPALIVE)", strerror(errno));
        return -1;
    }

    // Setting up the server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT);

    // Binding a socket to an address
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        syslog_log(LOG_ERR, __func__, "strerror", "Error binding a socket", strerror(errno));
        return -1;
    }

    // Listen on socket
    if (listen(server_socket, 5) == -1)
    {
        syslog_log(LOG_ERR, __func__, "strerror", "Error listen on socket", strerror(errno));
        return -1;
    }

    return server_socket;
}
int read_data_from_client(int client_socket)
{
    int received_bytes, client_id;

    // Getting the client ID
    received_bytes = recv(client_socket, client_id, sizeof(int), 0);
    if (received_bytes == -1)
    {
        syslog_log(LOG_ERR, __func__, "strerror", "Error receive from client", strerror(errno));
        return -1;
    }

    // Checking the correctness of the received data
    if (received_bytes != sizeof(int))
    {
        syslog_log(LOG_ERR, __func__, "strerror", "Error Invalid data format from client", strerror(errno));
        return -1;
    }
    printf("Received client ID for deletion: %d\n", client_id);
    deleteModel(client_id);
    DB_delete(client_id);
    return 0;
}