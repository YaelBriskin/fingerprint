#include "main.h"

int socket;
DataBase DB;

void setSystemTime(const char* dateTime) 
{
    char command[50];
    snprintf(command, sizeof(command), "sudo date -s '%s'", dateTime);

    // Execute the command
    int result = system(command);

    if (result != 0) {
        fprintf(stderr, "Error setting system time\n");
        // Handle the error as needed
    } else {
        printf("System time updated successfully\n");
    }
}

int main() 
{
    // Initialize I2C Display
    lcd16x2_i2c_init();
    lcd16x2_i2c_clear();
	lcd16x2_i2c_setCursor(0, 0);
	lcd16x2_i2c_printf("Welcome");
    //create socket
    socket =connectToServer();
    //create or open database
    DB= DB_open();

    char receivedDateTime[30];
    memset(receivedDateTime, 0, sizeof(receivedDateTime));
    // Receive date and time data from the server
    receiveDateTime(client_socket, receivedDateTime, sizeof(receivedDateTime));
    setSystemTime(receivedDateTime);

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