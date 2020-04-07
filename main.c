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
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }

}