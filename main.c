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

int keybord_fd, fpm_fd;
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
bool keyboard_initialized = false;
bool fpm_initialized = false;

char inputSequence[SEQ_LENGTH] = {0}; // Buffer size is 2
int seqIndex = 0;

/**
 * @brief Checks if the entered sequence matches the required sequence for a new employee.
 *
 * This function maintains a buffer of the last few entered characters and compares them
 * against the predefined sequence for adding a new employee, which is "*#". If the
 * sequence matches, the function returns true and resets the buffer. If the sequence
 * does not match, the function shifts the buffer to accommodate new input and
 * continues checking.
 *
 * @param input The current character entered by the user.
 * @return True if the sequence matches "*#", false otherwise.
 */
bool checkNewEmployeeSequence(char input)
{
  // Append the new input to the sequence buffer
  if (seqIndex < SEQ_LENGTH)
  {
    inputSequence[seqIndex++] = input;
  }

  // Check if the buffer contains the required sequence
  if (seqIndex == SEQ_LENGTH)
  {
    // Define the sequence for the new employee command
    const char *newEmployeeSequence = "*#";

    if (strncmp(inputSequence, newEmployeeSequence, SEQ_LENGTH) == 0)
    {
      // If sequence is matched, reset the buffer and index
      seqIndex = 0;
      return true;
    }

    // If the sequence is not matched, remove the oldest character
    memmove(inputSequence, inputSequence + 1, SEQ_LENGTH - 1);
    seqIndex--;
  }

  return false;
}
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
  if (keyboard_initialized)
  {
    close(keybord_fd);
  }
  if (fpm_initialized)
  {
    close(fpm_fd);
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
    LOG_MESSAGE(LOG_ERR, __func__, "strerror", "GPIO BUTTON IN initialization failed", strerror(errno));
    cleanup_resources();
    return FAILED;
  }
  gpio_button_in_initialized = true;

  if (GPIO_init(GPIO_BUTTON_OUT, "in") != SUCCESS)
  {
    LOG_MESSAGE(LOG_ERR, __func__, "strerror", "GPIO BUTTON OUT initialization failed", strerror(errno));
    cleanup_resources();
    return FAILED;
  }
  gpio_button_out_initialized = true;

  if (GPIO_init(GPIO_BUTTON_NEW, "in") != SUCCESS)
  {
    LOG_MESSAGE(LOG_ERR, __func__, "strerror", "GPIO BUTTON NEW initialization failed", strerror(errno));
    cleanup_resources();
    return FAILED;
  }
  gpio_button_new_initialized = true;

  if (GPIO_init(GPIO_BUZZER, "out") != SUCCESS)
  {
    LOG_MESSAGE(LOG_ERR, __func__, "strerror", "GPIO BUZZER initialization failed", strerror(errno));
    cleanup_resources();
    return FAILED;
  }
  gpio_buzzer_initialized = true;

  if (GPIO_init(GPIO_LED_RED, "out") != SUCCESS)
  {
    LOG_MESSAGE(LOG_ERR, __func__, "strerror", "GPIO LED RED initialization failed", strerror(errno));
    cleanup_resources();
    return FAILED;
  }
  gpio_led_red_initialized = true;

  // Initialize LCD
  if (!lcd20x4_i2c_init())
  {
    LOG_MESSAGE(LOG_ERR, __func__, "stderr", "LCD initialization failed!", NULL);
    cleanup_resources();
    return FAILED;
  }
  lcd_initialized = true;

  // Initialize UARTs
  keybord_fd = UART_Init(KEYBOARD, KEYBOARD_BaudRate);
  if (keybord_fd < 1)
  {
    LOG_MESSAGE(LOG_ERR, __func__, "strerror", "keyboard initialization failed", strerror(errno));
    cleanup_resources();
    return FAILED;
  }
  keyboard_initialized = true;

  fpm_fd = UART_Init(FPM_DEVICE, FPM_BaudRate);
  if (fpm_fd < 1)
  {
    LOG_MESSAGE(LOG_ERR, __func__, "strerror", "FPM initialization failed", strerror(errno));
    cleanup_resources();
    return FAILED;
  }
  fpm_initialized = true;

  return SUCCESS;
}

/**
 * @brief Handles the fingerprint input processing for different actions:
 *        - Entry (`1`)
 *        - Exit (`3`)
 *        - New Employee (`*#`)
 *
 * This function reads input from the UART, processes it based on the action specified, and
 * performs corresponding operations such as scanning fingerprints, updating the database,
 * and interacting with the LCD and buzzer.
 */
void fingerPrint()
{
  int8_t rx_buffer;
  char input;
  int id;
  // Read from UART to get the pressed key
  if (UART_read(keybord_fd, &rx_buffer, 1) == SUCCESS)
  {
    input = convert_to_char(rx_buffer); // Convert UART value to character
  }
  switch (input)
  {
  // Check if the IN button is pressed
  case '1':
    // Lock mutex to access shared resources safely
    if (pthread_mutex_lock(&displayMutex) != MUTEX_OK)
    {
      LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error locking mutex", strerror(errno));
      return;
    }
    displayLocked = LOCK;
    // Perform fingerprint scan for IN button
    id = findFinger(HELLO);
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
      LOG_MESSAGE(LOG_DEBUG, __func__, "stderr", "No matching in the library", NULL);
      sleep(SLEEP_LCD); // Wait before clearing the display
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
          LOG_MESSAGE(LOG_DEBUG, __func__, "stderr", mydata, NULL);
          buzzer(); // Activate the buzzer for successful entry
        }
        else
        {
          lcd20x4_i2c_puts(1, 0, "Failed to write to database"); // show on LCD
          LOG_MESSAGE(LOG_DEBUG, __func__, "stderr", "Failed to write to database", NULL);
        }
        sleep(SLEEP_LCD);
      }
      else if (result == 0 && id != CANCEL)
      {
        char mydata[23] = {0};
        sprintf(mydata, "The entered ID  #%d does not exist", id);
        lcd20x4_i2c_puts(0, 0, mydata);
        LOG_MESSAGE(LOG_DEBUG, __func__, "stderr", mydata, NULL);
        sleep(SLEEP_LCD);
      }
    }
    displayLocked = UNLOCK;
    // Send a signal to finish working with the display
    if (pthread_cond_signal(&displayCond) != SIGNAL_OK)
    {
      LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error signaling condition variable", strerror(errno));
    }
    lcd20x4_i2c_clear();
    if (pthread_mutex_unlock(&displayMutex) != MUTEX_OK)
    {
      LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error unlocking mutex", strerror(errno));
      return;
    }
    break;
    // Check if the OUT button is pressed
  case '3':

    if (pthread_mutex_lock(&displayMutex) != MUTEX_OK)
    {
      LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error locking mutex", strerror(errno));
      return;
    }
    displayLocked = LOCK;
    id = findFinger(GOODBYE);           // scan fingerprint
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
      LOG_MESSAGE(LOG_DEBUG, __func__, "stderr", "No matching in the library", NULL);
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
          LOG_MESSAGE(LOG_DEBUG, __func__, "OK", mydata, NULL);
          buzzer(); // turn on the buzzer
        }
        else
        {
          lcd20x4_i2c_puts(1, 0, "Failed to write to database"); // show on LCD
          LOG_MESSAGE(LOG_DEBUG, __func__, "stderr", "Failed to write to database", NULL);
        }
        sleep(SLEEP_LCD);
      }
      else if (result == 0 && id != CANCEL)
      {
        char mydata[23] = {0};
        sprintf(mydata, "The entered ID  #%d does not exist", id);
        lcd20x4_i2c_puts(0, 0, mydata);
        LOG_MESSAGE(LOG_DEBUG, __func__, "OK", mydata, NULL);
        sleep(SLEEP_LCD);
      }
    }
    displayLocked = UNLOCK;
    // Send a signal to finish working with the display
    if (pthread_cond_signal(&displayCond) != SIGNAL_OK)
    {
      LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error signaling condition variable", strerror(errno));
    }
    lcd20x4_i2c_clear();
    if (pthread_mutex_unlock(&displayMutex) != MUTEX_OK)
    {
      LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error unlocking mutex", strerror(errno));
      return;
    }
    break;
    // Start of NEW employee sequence
  case '*':
    // Reset buffer for new employee sequence check
    seqIndex = 0;
    inputSequence[seqIndex++] = input;
    break;
    // Check if NEW employee sequence is complete
  case '#':
    inputSequence[seqIndex++] = input;
    if (checkNewEmployeeSequence(input))
    {
      // Handle the NEW employee action
      int id = getNextAvailableID(); // get next ID value
      if (pthread_mutex_lock(&displayMutex) != MUTEX_OK)
      {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error locking mutex", strerror(errno));
        return;
      }
      displayLocked = LOCK;
      int ack = enrolling(id); // register a new fingerprint
      if (ack == 1)
      {
        DB_newEmployee(); // add a new employee to the database
        char messageString[MESSAGE_LEN];
        sprintf(messageString, "Employee %d added successfully.", id);
        lcd20x4_i2c_puts(0, 0, messageString); // show on LCD
        LOG_MESSAGE(LOG_DEBUG, __func__, "OK", messageString, NULL);
        buzzer();                               // turn on the buzzer
        timestamp = getCurrent_UTC_Timestamp(); // get current date and time in UTC format
        // send to the CRM ID of a new employee
        //  Retry sending JSON data if it fails
        int retries = 0;
        while (retries < g_max_retries && send_json_new_employee(id, timestamp) != SUCCESS)
        {
          retries++;
          char log_message[MAX_LOG_MESSAGE_LENGTH];
          snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Failed to send new employee data, retrying... %d", strerror(errno), retries);
          LOG_MESSAGE(LOG_ERR, __func__, "stderr", log_message, NULL);
          sleep(1); // Add some delay between retries if necessary
        }
        if (retries == g_max_retries)
        {
          LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Failed to send new employee data after retries", strerror(errno));
          deleteModel(id);
          DB_delete(id);
          lcd20x4_i2c_puts(0, 0, " Connection error. The new employee ID were not saved"); // show on LCD
          LOG_MESSAGE(LOG_DEBUG, __func__, "stderr", "Connection error. The new employee ID were not saved", NULL);
        }
        sleep(SLEEP_LCD);
      }
      else
      {
        // If enrolling fails, remove the entry from the database
        LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Enrolling failed.", NULL);
        lcd20x4_i2c_puts(1, 0, "Enrolling failed."); // show on LCD
        LOG_MESSAGE(LOG_DEBUG, __func__, "stderr", "Enrolling failed.", NULL);
      }

      displayLocked = UNLOCK;
      // Signal condition variable to indicate that display operation is complete
      if (pthread_cond_signal(&displayCond) != SIGNAL_OK)
      {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error signaling condition variable", strerror(errno));
      }
      lcd20x4_i2c_clear(); // Clear the display
      if (pthread_mutex_unlock(&displayMutex) != MUTEX_OK)
      {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error unlocking mutex", strerror(errno));
        return;
      }
    }
    break;

  default:
    break;
  }
  // Delay to debounce button presses
  usleep(DELAY);
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
  strncpy(g_database_path, config.database_path, MAX_PATH_LENGTH);

  // Initialize all peripherals and check for initialization failure
  int retries = 0;

  while (retries < g_max_retries && init() != SUCCESS)
  {
    retries++;
    // Log the retry attempt
    char log_message[MAX_LOG_MESSAGE_LENGTH];
    snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Initialization failed, retrying... %d", strerror(errno), retries);
    LOG_MESSAGE(LOG_ERR, __func__, "stderr", log_message, NULL);
    sleep(1); // Add some delay between retries if necessary
  }
  if (retries == g_max_retries)
  {
    LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Failed to initialize resources after retries", strerror(errno));
    syslog_close();
    return EXIT_FAILURE;
  }
  // Initialize the cURL library globally
  if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
  {
    LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Could not initialize cURL", strerror(errno));
    return EXIT_FAILURE;
  }
  //emptyDatabase(); // do this to empty database in FPM

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
    LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error creating displayThread thread", strerror(errno));
    curl_global_cleanup();
    return THREAD_ERROR;
  }
  if (pthread_create(&thread_database, NULL, databaseThread, NULL) != THREAD_OK)
  {
    LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error creating databaseThread thread", strerror(errno));
    curl_global_cleanup();
    return THREAD_ERROR;
  }
  if (pthread_create(&thread_deletion, NULL, post_requestThread, NULL) != THREAD_OK)
  {
    LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error creating post_requestThread thread", strerror(errno));
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

  // Cleanup cURL library globally
  curl_global_cleanup();

  return EXIT_SUCCESS;
}
