#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>

#include "./Inc/defines.h"
#include "./Inc/GPIO.h"
#include "./Inc/UART.h"
#include "./Inc/I2C.h"
#include "./Inc/DataBase.h"
#include "./Inc/lcd20x4_i2c.h"
#include "./Inc/threads.h"
#include "./Inc/config.h"
#include "./Inc/syslog_util.h"
#include "./Inc/signal_handlers.h"

// External declarations of condition variables
extern pthread_cond_t displayCond;
extern pthread_cond_t databaseCond;
extern pthread_cond_t requestCond;

// External declarations of mutexes
extern pthread_mutex_t displayMutex;
extern pthread_mutex_t databaseMutex;
extern pthread_mutex_t requestMutex;

extern pthread_mutex_t sqlMutex;
// External declarations of file
extern FILE *file_URL;

int uart2_fd, uart4_fd;
// Flag to stop threads
volatile sig_atomic_t stop = 0;

pthread_t thread_datetime, thread_database, thread_deletion;

//-------------display
pthread_mutex_t displayMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t displayCond = PTHREAD_COND_INITIALIZER;
int displayLocked = UNLOCK;
int timestamp;

// Global flags for tracking initialization
bool gpio_button_in_initialized = false;
bool gpio_button_out_initialized = false;
bool gpio_button_new_initialized = false;
bool gpio_buzzer_initialized = false;
bool gpio_led_red_initialized = false;
bool lcd_initialized = false;
bool uart2_initialized = false;
bool uart4_initialized = false;

/**
 * @brief Cleans up resources that have been initialized.
 *
 * This function checks which resources have been successfully initialized
 * and releases them accordingly. It helps to ensure that resources are
 * properly cleaned up in case of initialization failures.
 */
void cleanup_resources()
{
  // Clean up GPIOs
  if (gpio_button_in_initialized)
  {
    GPIO_close(GPIO_BUTTON_IN);
  }
  if (gpio_button_out_initialized)
  {
    GPIO_close(GPIO_BUTTON_OUT);
  }
  if (gpio_button_new_initialized)
  {
    GPIO_close(GPIO_BUTTON_NEW);
  }
  if (gpio_buzzer_initialized)
  {
    GPIO_close(GPIO_BUZZER);
  }
  if (gpio_led_red_initialized)
  {
    GPIO_close(GPIO_LED_RED);
  }
  // Clean up LCD
  if (lcd_initialized)
  {
    I2C_close();
  }
  // Clean up UARTs
  if (uart2_initialized)
  {
    close(uart2_fd);
  }
  if (uart4_initialized)
  {
    close(uart4_fd);
  }
}

/**
 * @brief Initialize hardware and other resources.
 *
 * This function initializes all required hardware components, including GPIOs,
 * LCD, and UARTs. If any component fails to initialize, it logs the error,
 * cleans up already initialized resources, and returns a failure status.
 *
 * @return Status of the initialization. Returns SUCCESS if all components are
 *         initialized successfully, otherwise returns FAILED.
 */
Status_t init()
{
  // Initialize GPIOs
  if (GPIO_init(GPIO_BUTTON_IN, "in") != SUCCESS)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "GPIO BUTTON IN initialization failed", strerror(errno));
    cleanup_resources();
    return FAILED;
  }
  gpio_button_in_initialized = true;

  if (GPIO_init(GPIO_BUTTON_OUT, "in") != SUCCESS)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "GPIO BUTTON OUT initialization failed", strerror(errno));
    cleanup_resources();
    return FAILED;
  }
  gpio_button_out_initialized = true;

  if (GPIO_init(GPIO_BUTTON_NEW, "in") != SUCCESS)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "GPIO BUTTON NEW initialization failed", strerror(errno));
    cleanup_resources();
    return FAILED;
  }
  gpio_button_new_initialized = true;

  if (GPIO_init(GPIO_BUZZER, "out") != SUCCESS)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "GPIO BUZZER initialization failed", strerror(errno));
    cleanup_resources();
    return FAILED;
  }
  gpio_buzzer_initialized = true;

  if (GPIO_init(GPIO_LED_RED, "out") != SUCCESS)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "GPIO LED RED initialization failed", strerror(errno));
    cleanup_resources();
    return FAILED;
  }
  gpio_led_red_initialized = true;

  // Initialize LCD
  if (!lcd20x4_i2c_init())
  {
    syslog_log(LOG_ERR, __func__, "strerr", "LCD initialization failed!", NULL);
    cleanup_resources();
    return FAILED;
  }
  lcd_initialized = true;

  // Initialize UARTs
  uart2_fd = UART_Init(UART2_DEVICE, UART2_BaudRate);
  if (uart2_fd < 1)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "UART2 initialization failed", strerror(errno));
    cleanup_resources();
    return FAILED;
  }
  uart2_initialized = true;

  uart4_fd = UART_Init(UART4_DEVICE, UART4_BaudRate);
  if (uart4_fd < 1)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "UART4 initialization failed", strerror(errno));
    cleanup_resources();
    return FAILED;
  }
  uart4_initialized = true;

  return SUCCESS;
}

/**
 * @brief Handle fingerprint operations based on button presses.
 *
 * This function continuously checks the state of three GPIO buttons:
 * IN, OUT, and NEW. Depending on which button is pressed, it performs
 * different actions related to fingerprint recognition and database
 * operations. It uses mutexes to ensure thread safety when accessing
 * shared resources like the display.
 */
void fingerPrint()
{
  // Open GPIOs for buttons
  int button_fd_in = GPIO_open(GPIO_BUTTON_IN, O_RDONLY);
  int button_fd_out = GPIO_open(GPIO_BUTTON_OUT, O_RDONLY);
  int button_fd_new = GPIO_open(GPIO_BUTTON_NEW, O_RDONLY);
  if (button_fd_in == ERROR || button_fd_out == ERROR || button_fd_new == ERROR)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "Error opening GPIO value file", strerror(errno));
    return;
  }
  // Check if the IN button is pressed
  if (!GPIO_read(button_fd_in))
  {
    // Lock mutex to access shared resources safely
    if (pthread_mutex_lock(&displayMutex) != MUTEX_OK)
    {
      syslog_log(LOG_ERR, __func__, "strerror", "Error locking mutex", strerror(errno));
      return;
    }
    displayLocked = LOCK;
    // Perform fingerprint scan for IN button
    int id = findFinger(HELLO);
    timestamp = getCurrent_UTC_Timestamp(); // get current date and time in UTC format
    if (id > 0)
    {
      buzzer();         // Activate the buzzer for successful scan
      sleep(SLEEP_LCD); // Wait before updating the display

      // Write entry to the database
      for (int attempts = 0; attempts < g_max_retries; attempts++)
      {
        if (DB_write(id, timestamp, IN, TRUE) == SUCCESS)
          break;
      }
    }
    else if (id == -1)
    {
      lcd20x4_i2c_puts(1, 0, "No matching in the library"); // show on LCD
      sleep(SLEEP_LCD);                                     // Wait before clearing the display
    }
    else
    {
      // Handle case where fingerprint is not found in the database
      id = enter_ID_keypad(); // Prompt user to enter ID via keypad
      int result = DB_check_id_exists(id);
      if (id > 0 && result)
      {
        if (DB_write(id, timestamp, IN, FALSE) == SUCCESS) // write to database
        {
          char mydata[23] = {0};
          sprintf(mydata, "Hello  ID #%d", id);
          lcd20x4_i2c_puts(1, 0, mydata); // show on LCD
          buzzer();                       // Activate the buzzer for successful entry
        }
        else
        {
          lcd20x4_i2c_puts(1, 0, "Failed to write to database"); // show on LCD
        }
        sleep(SLEEP_LCD);
      }
      else if (result == 0)
      {
        char mydata[23] = {0};
        sprintf(mydata, "The entered ID  #%d does not exist", id);
        lcd20x4_i2c_puts(0, 0, mydata);
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
      return;
    }
  }
  // Check if the OUT button is pressed
  if (!GPIO_read(button_fd_out))
  {
    if (pthread_mutex_lock(&displayMutex) != MUTEX_OK)
    {
      syslog_log(LOG_ERR, __func__, "strerror", "Error locking mutex", strerror(errno));
      return;
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
      int result = DB_check_id_exists(id);
      if (id > 0 && result)
      {
        if (DB_write(id, timestamp, OUT, FALSE) == SUCCESS) // write to database
        {
          char mydata[23] = {0};
          sprintf(mydata, "Goodbye  ID #%d", id);
          lcd20x4_i2c_puts(1, 0, mydata); // show on LCD
          buzzer();                       // turn on the buzzer
        }
        else
        {
          lcd20x4_i2c_puts(1, 0, "Failed to write to database"); // show on LCD
        }
        sleep(SLEEP_LCD);
      }
      else if (result == 0)
      {
        char mydata[23] = {0};
        sprintf(mydata, "The entered ID  #%d does not exist", id);
        lcd20x4_i2c_puts(0, 0, mydata);
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
      return;
    }
  }
  // press button NEW (new employee)
  if (!GPIO_read(button_fd_new))
  {
    int id = getNextAvailableID(); // get next ID value
    if (pthread_mutex_lock(&displayMutex) != MUTEX_OK)
    {
      syslog_log(LOG_ERR, __func__, "strerror", "Error locking mutex", strerror(errno));
      return;
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
    // Signal condition variable to indicate that display operation is complete
    if (pthread_cond_signal(&displayCond) != SIGNAL_OK)
    {
      syslog_log(LOG_ERR, __func__, "strerror", "Error signaling condition variable", strerror(errno));
    }
    lcd20x4_i2c_clear(); // Clear the display
    if (pthread_mutex_unlock(&displayMutex) != MUTEX_OK)
    {
      syslog_log(LOG_ERR, __func__, "strerror", "Error unlocking mutex", strerror(errno));
      return;
    }
  }
  // Delay to debounce button presses
  usleep(DELAY);
  // Close GPIOs after operations
  GPIO_close(GPIO_BUTTON_IN);
  GPIO_close(GPIO_BUTTON_OUT);
  GPIO_close(GPIO_BUTTON_NEW);
}

/**
 * @brief Main entry point of the program.
 * @return Exit status.
 */
int main()
{
  Config_t config;

  // syslog initialization
  syslog_init();
  // signal to kill programm (Ctrl+C)
  setup_sigint_handler();

  // Read all data from config file
  if (read_config(&config) == FAILED)
  {
    syslog_close();
    return 1;
  }
  // Initialize global variables from config file
  g_server_port = config.server_port;
  g_month = config.month;
  strncpy(g_url, config.url, MAX_URL_LENGTH);
  strncpy(g_url_new_employee, config.url_new_employee, MAX_URL_LENGTH);
  strncpy(g_url_delete_employee, config.url_delete_employee, MAX_URL_LENGTH);
  strncpy(g_url_check_delete, config.url_check_delete, MAX_URL_LENGTH);
  strncpy(g_header, config.header, MAX_HEADER_LENGTH);
  g_max_retries = config.max_retries;
  g_db_sleep = config.db_sleep;
  strncpy(g_lcd_message, config.lcd_message, MAX_LCD_MESSAGE_LENGTH);

  // Initialize all peripherals and check for initialization failure
  int retries = 0;

  while (retries < g_max_retries && init() != SUCCESS)
  {
    retries++;
    // Log the retry attempt
    syslog_log(LOG_ERR, __func__, "strerror", "Initialization failed, retrying... %d", strerror(errno), retries);
    sleep(1); // Add some delay between retries if necessary
  }
  if (retries == g_max_retries)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "Failed to initialize resources after retries", strerror(errno));
    syslog_close();
    return EXIT_FAILURE;
  }
  // Initialize the cURL library globally
  if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "Could not initialize cURL", strerror(errno));
    return EXIT_FAILURE;
  }
  // emptyDatabase(); //do this to empty database in FPM

  // create or open database
  DB_open();

  // Turn off LED
  int fd_led = GPIO_open(GPIO_LED_RED, O_WRONLY);
  GPIO_write(fd_led, LED_OFF);
  GPIO_close(fd_led);

  // Initialize the file using the name from the configuration
  initFile(&file_URL, FILE_NAME);

  // Create a threads
  if (pthread_create(&thread_datetime, NULL, clockThread, NULL) != THREAD_OK)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "Error creating displayThread thread", strerror(errno));
    curl_global_cleanup();
    return THREAD_ERROR;
  }
  if (pthread_create(&thread_database, NULL, databaseThread, NULL) != THREAD_OK)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "Error creating databaseThread thread", strerror(errno));
    curl_global_cleanup();
    return THREAD_ERROR;
  }
  if (pthread_create(&thread_deletion, NULL, post_requestThread, NULL) != THREAD_OK)
  {
    syslog_log(LOG_ERR, __func__, "strerror", "Error creating post_requestThread thread", strerror(errno));
    curl_global_cleanup();
    return THREAD_ERROR;
  }
  while (!stop)
  {
    fingerPrint();
  }
  // Wait for the thread to complete
  pthread_join(thread_datetime, NULL);
  pthread_join(thread_database, NULL);
  pthread_join(thread_deletion, NULL);

   pthread_cond_destroy(&databaseCond);
    pthread_cond_destroy(&displayCond);
    pthread_mutex_destroy(&sqlMutex);
    pthread_mutex_destroy(&databaseMutex);
    pthread_mutex_destroy(&displayMutex);

    curl_global_cleanup();
    DB_close();
    UART_close(uart2_fd);
    UART_close(uart4_fd);
    I2C_close();
    closeFile(file_URL);

    syslog_close();

  return EXIT_SUCCESS;
}
