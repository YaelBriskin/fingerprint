#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "./Inc/GPIO.h"
#include "./Inc/UART.h"
#include "./Inc/I2C.h"
#include "./Inc/client_socket_http.h"
#include "./Inc/DataBase.h"
#include "./Inc/lcd20x4_i2c.h"
#include "./Inc/threads.h"
#include <stdbool.h>
#include <time.h>

volatile bool isRunning = true;

int main()
{
  pthread_t thread_datetime, thread_FPM;
  // Initialize I2C Display
  if (lcd20x4_i2c_init())
  {
    printf("LCD initialization successful!\n");
    // Create a threads
    if (pthread_create(&thread_datetime, NULL, displayThread, NULL) != 0)
    {
      fprintf(stderr, "Error creating thread\n");
      return 1;
    }
    if (pthread_create(&thread_FPM, NULL, fingerPrintThread, NULL) != 0)
    {
      fprintf(stderr, "Error creating thread\n");
      return 1;
    }
  }
  else
    printf("LCD initialization failed!\n");
  // create or open database
  DB_open();
  //sleep(10);
  // isRunning = false;

  // Ожидаем завершения потока
  pthread_join(thread_datetime, NULL);
  pthread_join(thread_FPM, NULL);
}
