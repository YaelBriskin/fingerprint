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
#include "./Inc/config.h"
#include "./Inc/syslog_util.h"
#include "./Inc/defines.h"

#include <stdbool.h>
#include <time.h>

volatile bool isRunning = true;
int uart2_fd, uart4_fd;

Status_t init()
{
  if (GPIO_init(GPIO_BUTTON_IN, "in") != SUCCESS || GPIO_init(GPIO_BUTTON_OUT, "in") != SUCCESS || GPIO_init(GPIO_BUTTON_NEW, "in") != SUCCESS)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "GPIO BUTTON initialization failed", strerror(errno));
    return FAILED;
  }
  if (GPIO_init(GPIO_BUZZER, "out") != SUCCESS)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "GPIO BUZZER initialization failed", strerror(errno));
    return FAILED;
  }
  if (GPIO_init(GPIO_LED_RED, "out") != SUCCESS)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "GPIO LED initialization failed", strerror(errno));
    return FAILED;
  }
  if (!lcd20x4_i2c_init())
  {
    syslog_log(LOG_ERR, __func__, "strerr", "LCD initialization failed!", NULL);
    return FAILED;
  }
  uart2_fd = UART_Init(UART2_DEVICE, UART2_BaudRate);
  uart4_fd = UART_Init(UART4_DEVICE, UART4_BaudRate);
  if (uart2_fd < 1)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "UART2 initialization failed", strerror(errno));
    return FAILED;
  }
  if (uart4_fd < 1)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "UART4 initialization failed", strerror(errno));
    return FAILED;
  }
  printf("GPIO initialization successful!\n");
  printf("LCD initialization successful!\n");
  printf("UART initialization successful!\n");
  return SUCCESS;
}
int main()
{
  Config_t config;
  pthread_t thread_datetime, thread_FPM, thread_database, thread_socket;
  syslog_init();
    // Initialize I2C Display
  if (init() == SUCCESS && read_config(&config) == SUCCESS)
  {
    // Initialize the cURL library globally
    if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) 
    {
        syslog_log(LOG_ERR, __func__, "strerror", "Could not initialize cURL", strerror(errno));
        return EXIT_FAILURE;
    }
    emptyDatabase();
    // Initialize global variables
    g_server_port = config.server_port;
    g_month = config.month;
    strncpy(g_url, config.url, MAX_URL_LENGTH);
    strncpy(g_url_new_employee, config.url_new_employee, MAX_URL_LENGTH);
    strncpy(g_header, config.header, MAX_HEADER_LENGTH);
    g_max_retries = config.max_retries;

    // create or open database
    DB_open();

    int fd_led = GPIO_open(GPIO_LED_RED,O_WRONLY);
    GPIO_write(fd_led,0);
    GPIO_close(fd_led);
    // Create a threads
    if (pthread_create(&thread_datetime, NULL, clockThread, NULL) != THREAD_OK)
    {
      syslog_log(LOG_ERR, __func__, "strerror", "Error creating displayThread thread", strerror(errno));
      curl_global_cleanup();
      return THREAD_ERROR;
    }
    if (pthread_create(&thread_FPM, NULL, fingerPrintThread, NULL) != THREAD_OK)
    {
      syslog_log(LOG_ERR, __func__, "strerror", "Error creating fingerPrintThread thread", strerror(errno));
      curl_global_cleanup();
      return THREAD_ERROR;
    }
    if (pthread_create(&thread_database, NULL, databaseThread, NULL) != THREAD_OK)
    {
      syslog_log(LOG_ERR, __func__, "strerror", "Error creating displayThread thread", strerror(errno));
      curl_global_cleanup();
      return THREAD_ERROR;
    }
    if (pthread_create(&thread_socket, NULL, socket_serverThread, NULL) != THREAD_OK) 
    {
      syslog_log(LOG_ERR, __func__, "strerror", "Error creating socketThread thread", strerror(errno));
      curl_global_cleanup();
      return THREAD_ERROR;
    }
    // Wait for the thread to complete
    pthread_join(thread_datetime, NULL);
    pthread_join(thread_FPM, NULL);
    pthread_join(thread_database, NULL);
    pthread_join(thread_socket, NULL);

    // Cleanup cURL library globally
    curl_global_cleanup();

    return EXIT_SUCCESS;
  }
  else
    syslog_close();
    return EXIT_FAILURE;
}
