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
#include "./Inc/lcd20x4_i2c.h"
#include "./Inc/threads.h"
#include <stdbool.h>
#include <time.h>

void *displayThread(void *arg);

volatile bool isRunning = true;

void *displayThread(void *arg) 
{
  printf("in thread\r\n");
  while(isRunning)
  {
    // Получение текущего времени
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // Форматирование времени в нужный вид
    char timeString[20] = {'\0'};
    strftime(timeString, 20, "%X %x", timeinfo);

    // Вывод на дисплей
    lcd20x4_i2c_puts(0,0,timeString);
    sleep(1);
  }
    return NULL;
}

int main()
{
    pthread_t thread_datetime;
    //configureNetwork();
    //restartNetworking();
    // Initialize I2C Display
    if (lcd20x4_i2c_init()) 
    {
        // Создаем поток
        if (pthread_create(&thread_datetime, NULL, displayThread, NULL) != 0) {
          fprintf(stderr, "Error creating thread\n");
          return 1;
       }        
    }    else 
        printf("LCD initialization failed!\n");
    //create socket
  //  connectToServer();
    //create or open database
  //  DB_open();
       // sleep(10);
       // isRunning = false;

        // Ожидаем завершения потока
       // pthread_join(thread_datetime, NULL);
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
   pthread_join(thread_datetime, NULL);
       return 0;

}