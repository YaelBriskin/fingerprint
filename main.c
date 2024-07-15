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
  return SUCCESS;
}
void fingerPrint()
{
      int button_fd_in = GPIO_open(GPIO_BUTTON_IN, O_RDONLY);
      int button_fd_out = GPIO_open(GPIO_BUTTON_OUT, O_RDONLY);
      int button_fd_new = GPIO_open(GPIO_BUTTON_NEW, O_RDONLY);
      if (button_fd_in == ERROR || button_fd_out == ERROR || button_fd_new == ERROR)
      {
        syslog_log(LOG_ERR, __func__, "strerror", "Error opening GPIO value file", strerror(errno));
        break;
      }
      // press button IN
      if (!GPIO_read(button_fd_in))
      {
        if (pthread_mutex_lock(&displayMutex) != MUTEX_OK)
        {
          syslog_log(LOG_ERR, __func__, "strerror", "Error locking mutex", strerror(errno));
          break;
        }
        displayLocked = LOCK;
        int id = findFinger(HELLO);             // scan fingerprint
        timestamp = getCurrent_UTC_Timestamp(); // get current date and time in UTC format
        if (id > 0)
        {
          buzzer(); // turn on the buzzer
          // sleep(SLEEP_LCD);
          // write to database
          for (int attempts = 0; attempts < g_max_retries; attempts++)
          {
            if (DB_write(id, timestamp, IN, TRUE) == SUCCESS)
              break;
          }
        }
        else if (id == -1)
        {
          lcd20x4_i2c_puts(1, 0, "No matching in the library"); // show on LCD
          sleep(SLEEP_LCD);
        }
        else
        {
          id = enter_ID_keypad(); // enter ID using the keypad
          if (id > 0)
          {
            if (DB_write(id, timestamp, IN, FALSE) == SUCCESS) // write to database
            {
              char mydata[23] = {0};
              sprintf(mydata, "Hello  ID #%d", id);
              lcd20x4_i2c_puts(1, 0, mydata); // show on LCD
              buzzer();                       // turn on the buzzer
              sleep(SLEEP_LCD);
            }
            else
              lcd20x4_i2c_puts(1, 0, "Failed to write to database"); // show on LCD
            sleep(SLEEP_LCD);
          }
        }
        displayLocked = UNLOCK;
        // Send a signal to finish working with the display
        if (pthread_cond_signal(&displayCond) != SIGNAL_OK)
        {
          syslog_log(LOG_ERR, __func__, "strerror", "Error signaling condition variable", strerror(errno));
        }
        lcd20x4_i2c_clear();
        if (pthread_mutex_unlock(&displayMutex) != MUTEX_OK)
        {
          syslog_log(LOG_ERR, __func__, "strerror", "Error unlocking mutex", strerror(errno));
          break;
        }
      }
      // press button OUT
      if (!GPIO_read(button_fd_out))
      {
        if (pthread_mutex_lock(&displayMutex) != MUTEX_OK)
        {
          syslog_log(LOG_ERR, __func__, "strerror", "Error locking mutex", strerror(errno));
          break;
        }
        displayLocked = LOCK;
        int id = findFinger(GOODBYE);           // scan fingerprint
        timestamp = getCurrent_UTC_Timestamp(); // get current date and time in UTC format
        if (id > 0)
        {
          buzzer(); // turn on the buzzer
          // sleep(SLEEP_LCD);
          // write to database
          for (int attempts = 0; attempts < g_max_retries; attempts++)
          {
            if (DB_write(id, timestamp, OUT, TRUE) == SUCCESS)
              break;
          }
        }
        else if (id == -1)
        {
          lcd20x4_i2c_puts(1, 0, "No matching in the library"); // show on LCD
          sleep(SLEEP_LCD);
        }
        else
        {
          id = enter_ID_keypad(); // enter ID using the keypad
          if (id > 0)
          {
            if (DB_write(id, timestamp, OUT, FALSE) == SUCCESS) // write to database
            {
              char mydata[23] = {0};
              sprintf(mydata, "Goodbye  ID #%d", id);
              lcd20x4_i2c_puts(1, 0, mydata); // show on LCD
              buzzer();                       // turn on the buzzer
              sleep(SLEEP_LCD);
            }
            else
              lcd20x4_i2c_puts(1, 0, "Failed to write to database"); // show on LCD
            sleep(SLEEP_LCD);
          }
        }
        displayLocked = UNLOCK;
        // Send a signal to finish working with the display
        if (pthread_cond_signal(&displayCond) != SIGNAL_OK)
        {
          syslog_log(LOG_ERR, __func__, "strerror", "Error signaling condition variable", strerror(errno));
        }
        lcd20x4_i2c_clear();
        if (pthread_mutex_unlock(&displayMutex) != MUTEX_OK)
        {
          syslog_log(LOG_ERR, __func__, "strerror", "Error unlocking mutex", strerror(errno));
          break;
        }
      }
      // press button NEW (new employee)
      if (!GPIO_read(button_fd_new))
      {
        int id = getNextAvailableID(); // get next ID value
        if (pthread_mutex_lock(&displayMutex) != MUTEX_OK)
        {
          syslog_log(LOG_ERR, __func__, "strerror", "Error locking mutex", strerror(errno));
          break;
        }
        displayLocked = LOCK;
        int ack = enrolling(id); // register a new fingerprint
        if (ack == 1)
        {
          DB_newEmployee(); // add a new employee to the database
          char messageString[MESSAGE_LEN];
          sprintf(messageString, "Employee %d added successfully.", id);
          lcd20x4_i2c_puts(0, 0, messageString);  // show on LCD
          buzzer();                               // turn on the buzzer
          timestamp = getCurrent_UTC_Timestamp(); // get current date and time in UTC format
          // send to the CRM ID of a new employee
          //  Retry sending JSON data if it fails
          int retries = 0;
          while (retries < g_max_retries && send_json_new_employee(id, timestamp) != SUCCESS)
          {
            retries++;
            syslog_log(LOG_ERR, __func__, "strerror", "Failed to send new employee data, retrying... %d", strerror(errno), retries);
            sleep(1); // Add some delay between retries if necessary
          }
          if (retries == g_max_retries)
          {
            syslog_log(LOG_ERR, __func__, "strerror", "Failed to send new employee data after retries", strerror(errno));
            deleteModel(id);
            DB_delete(id);
            lcd20x4_i2c_puts(0, 0, " Connection error. The new employee ID were not saved"); // show on LCD
          }
          sleep(SLEEP_LCD);
        }
        else
        {
          // If enrolling fails, remove the entry from the database
          syslog_log(LOG_ERR, __func__, "format", "Enrolling failed.");
          lcd20x4_i2c_puts(1, 0, "Enrolling failed."); // show on LCD
        }

        displayLocked = UNLOCK;
        // Send a signal to finish working with the display
        if (pthread_cond_signal(&displayCond) != SIGNAL_OK)
        {
          syslog_log(LOG_ERR, __func__, "strerror", "Error signaling condition variable", strerror(errno));
        }
        lcd20x4_i2c_clear();
        if (pthread_mutex_unlock(&displayMutex) != MUTEX_OK)
        {
          syslog_log(LOG_ERR, __func__, "strerror", "Error unlocking mutex", strerror(errno));
          break;
        }
      }
      usleep(DELAY);
      GPIO_close(GPIO_BUTTON_IN);
      GPIO_close(GPIO_BUTTON_OUT);
      GPIO_close(GPIO_BUTTON_NEW);
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
    //
    // emptyDatabase();
    // Initialize global variables
    g_server_port = config.server_port;
    g_month = config.month;
    strncpy(g_url, config.url, MAX_URL_LENGTH);
    strncpy(g_url_new_employee, config.url_new_employee, MAX_URL_LENGTH);
    strncpy(g_header, config.header, MAX_HEADER_LENGTH);
    g_max_retries = config.max_retries;
    g_db_sleep = config.db_sleep;
    strncpy(g_file_name, config.file_name, MAX_FILENAME_LENGTH);
    // create or open database
    DB_open();

    int fd_led = GPIO_open(GPIO_LED_RED, O_WRONLY);
    GPIO_write(fd_led, 0);
    GPIO_close(fd_led);

    // Initialize the file using the name from the configuration
    initFile(config.file_name);
    // Create a threads
    if (pthread_create(&thread_datetime, NULL, clockThread, NULL) != THREAD_OK)
    {
      syslog_log(LOG_ERR, __func__, "strerror", "Error creating displayThread thread", strerror(errno));
      curl_global_cleanup();
      return THREAD_ERROR;
    }
    // if (pthread_create(&thread_FPM, NULL, fingerPrintThread, NULL) != THREAD_OK)
    // {
    //   syslog_log(LOG_ERR, __func__, "strerror", "Error creating fingerPrintThread thread", strerror(errno));
    //   curl_global_cleanup();
    //   return THREAD_ERROR;
    // }
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
    int timestamp;
    while (1)
    {
      fingerPrint();
    }

    // Wait for the thread to complete
    pthread_join(thread_datetime, NULL);
    //pthread_join(thread_FPM, NULL);
    pthread_join(thread_database, NULL);
    pthread_join(thread_socket, NULL);

    // Cleanup cURL library globally
    curl_global_cleanup(); 

    return EXIT_SUCCESS;
  }
  else
    syslog_close();
  closeFile();
  return EXIT_FAILURE;
}
