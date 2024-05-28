#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "./Inc/GPIO.h"
#include "./Inc/UART.h"
#include "./Inc/I2C.h"
#include "./Inc/DataBase.h"
#include "./Inc/lcd20x4_i2c.h"
#include "./Inc/threads.h"
#include "./Inc/syslog_util.h"
#include <stdbool.h>
#include <time.h>

volatile bool isRunning = true;
int uart2_fd, uart4_fd;

int init()
{
  if (GPIO_init(GPIO_BUTTON_IN, "in") != 1 || GPIO_init(GPIO_BUTTON_OUT, "in") != 1 || GPIO_init(GPIO_BUTTON_NEW, "in") != 1)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "GPIO BUTTON initialization failed", strerror(errno));
    return 0;
  }
  if (GPIO_init(GPIO_BUZZER, "out") != 1)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "GPIO BUZZER initialization failed", strerror(errno));
    return 0;
  }
  if (GPIO_init(GPIO_LED_RED, "out") != 1)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "GPIO LED initialization failed", strerror(errno));
    return 0;
  }
  if (!lcd20x4_i2c_init())
  {
    syslog_log(LOG_ERR, __func__, "strerr", "LCD initialization failed!", NULL);
    return 0;
  }
  uart2_fd = UART_Init(UART2_DEVICE, UART2_BaudRate);
  uart4_fd = UART_Init(UART4_DEVICE, UART4_BaudRate);
  if (uart2_fd < 1)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "UART2 initialization failed", strerror(errno));
    return 0;
  }
  if (uart4_fd < 1)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "UART4 initialization failed", strerror(errno));
    return 0;
  }
  printf("GPIO initialization successful!\n");
  printf("LCD initialization successful!\n");
  printf("UART initialization successful!\n");
  return 1;
}
int main()
{
  pthread_t thread_datetime, thread_FPM, thread_database, thread_socket;
  syslog_init();
    // Initialize I2C Display
  if (init())
  {
    // create or open database
    DB_open();
    int button_fd_led = GPIO_open(GPIO_LED_RED,O_WRONLY);
    GPIO_write(button_fd_led,0);
    // Create a threads
    if (pthread_create(&thread_datetime, NULL, clockThread, NULL) != 0)
    {
      syslog_log(LOG_ERR, __func__, "strerror", "Error creating displayThread thread", strerror(errno));
      return 1;
    }
    if (pthread_create(&thread_FPM, NULL, fingerPrintThread, NULL) != 0)
    {
      syslog_log(LOG_ERR, __func__, "strerror", "Error creating fingerPrintThread thread", strerror(errno));
      return 1;
    }
    if (pthread_create(&thread_database, NULL, databaseThread, NULL) != 0)
    {
      syslog_log(LOG_ERR, __func__, "strerror", "Error creating displayThread thread", strerror(errno));
      return 1;
    }
    if (pthread_create(&thread_socket, NULL, socket_serverThread, NULL) != 0) 
    {
      syslog_log(LOG_ERR, __func__, "strerror", "Error creating socketThread thread", strerror(errno));
      return 1;
  }
    // Wait for the thread to complete
    pthread_join(thread_datetime, NULL);
    pthread_join(thread_FPM, NULL);
    pthread_join(thread_database, NULL);
    pthread_join(thread_socket, NULL);
  }
  else

    syslog_close();
}
