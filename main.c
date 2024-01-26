#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "./Inc/GPIO.h"
#include "./Inc/UART.h"
#include "./Inc/I2C.h"
#include "./Inc/client_socket.h"
#include "./Inc/DataBase.h"
#include "./Inc/lcd16x2_i2c.h"
#include "./Inc/threads.h"

#include <time.h>

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

	lcd16x2_i2c_setCursor(1, 0);
    lcd16x2_i2c_printf("%s", timeString);
}

int main()
{
    printf("Hello from main\r\n");
    //configureNetwork();
    //restartNetworking();
    // Initialize I2C Display
    lcd16x2_i2c_init();
   // lcd16x2_i2c_clear();
	//lcd16x2_i2c_setCursor(0, 0);
	lcd16x2_i2c_printf("Welcome");
   // displayCurrentTime();
    //create socket
  //  connectToServer();
    //create or open database
  //  DB_open();

    // Create a threads
 //   pthread_t thread_fingerPrint,thread_database,thread_socket;
  //  if (pthread_create(&thread_fingerPrint, NULL, fingerPrintThread, NULL) != 0) 
 //   {
  //      perror("Error creating button thread");
 //       exit(EXIT_FAILURE);
  //  }
  //  if (pthread_create(&thread_database, NULL, databaseThread, NULL) != 0) 
 //   {
  //      perror("Error creating database thread");
  //      exit(EXIT_FAILURE);
  //  }
  //  if(pthread_create(&thread_socket, NULL, socketThread, NULL) != 0)
  //  {
  //      perror("Error creating database thread");
   //     exit(EXIT_FAILURE);
   // }
}