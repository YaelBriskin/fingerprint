#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "GPIO.h"
#include "/Inc/UART.h"
#include "I2C.h"
#include "client_socket.h"
#include "DataBase.h"
#include <time.h>

#define MAX_RETRIES 10
void displayCurrentTime() 
{
    // Выполнение команды "date" и считывание вывода
    FILE* dateCommand = popen("date", "r");
    if (dateCommand == NULL) {
        perror("date command error");
        exit(EXIT_FAILURE);
    }
    char timeString[50];
    fgets(timeString, sizeof(timeString), dateCommand);
    pclose(dateCommand);

    lcd16x2_i2c_printf("%s", timeString);
}

int main()
{
    //configureNetwork();
    //restartNetworking();
    // Initialize I2C Display
    lcd16x2_i2c_init();
    lcd16x2_i2c_clear();
	lcd16x2_i2c_setCursor(0, 0);
	lcd16x2_i2c_printf("Welcome");
    displayCurrentTime();
    //create socket
    connectToServer();
    //create or open database
    DB_open();

    char receivedDateTime[30];
    memset(receivedDateTime, 0, sizeof(receivedDateTime));
    // Receive date and time data from the server
    receiveDateTime(client_socket, receivedDateTime, sizeof(receivedDateTime));
    setSystemTime(receivedDateTime);

    // Create a threads
    pthread_t thread_fingerPrint,thread_database,thread_socket;
    if (pthread_create(&thread_fingerPrint, NULL, fingerPrintThread, NULL) != 0) 
    {
        perror("Error creating button thread");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&thread_database, NULL, databaseThread, NULL) != 0) 
    {
        perror("Error creating database thread");
        exit(EXIT_FAILURE);
    }
    if(pthread_create(&thread_socket, NULL, socketThread, NULL) != 0)
}