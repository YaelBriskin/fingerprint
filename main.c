#include "main.h"

int socket;
DataBase DB;
int main() 
{
    // Initialize I2C Display
    lcd16x2_i2c_init();
    //create socket
    socket =connectToServer();
    //create or open database
    DB= DB_open();
    // Create a thread for checking the button state
    pthread_t thread_button,thread_uart,thread_socket,thread_database;
    if (pthread_create(&thread_button, NULL, buttonThread, NULL) != 0) 
    {
        perror("Error creating button thread");
        exit(EXIT_FAILURE);
    }
        if (pthread_create(&thread_database, NULL, databaseThread, NULL) != 0) 
    {
        perror("Error creating database thread");
        exit(EXIT_FAILURE);
    }
        if (pthread_create(&thread_uart, NULL, uartThread, NULL) != 0) 
    {
        perror("Error creating uart thread");
        exit(EXIT_FAILURE);
    }

}