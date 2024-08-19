#ifndef DEFINES_H
#define DEFINES_H

#define CANCEL 0
#define ERROR -1
#define MUTEX_OK 0
#define MUTEX_ERROR 1
#define THREAD_OK 0
#define THREAD_ERROR 1
#define SIGNAL_OK 0
#define SIGNAL_ERROR 1
#define LED_ON 1
#define LED_OFF 0
#define SLEEP_BUZZER 500000
#define BUZZER_ON 1
#define BUZZER_OFF 0
#define LOCK 1
#define UNLOCK 0
#define SLEEP_LCD 3
#define SLEEP_DURATION 4
#define ONE_MINUTE 60 
#define MONTH 2
#define CHECK_INTERVAL (24 * 60 * 60) // 24 hours in seconds

#define TRUE "true"
#define FALSE "false"
#define IN "in"
#define OUT "out"
#define HELLO "Hello"
#define GOODBYE "Goodbye"
#define DELAY 10000
#define TIME_STR_LEN 20

#define MESSAGE_LEN 50

#define MAX_LOG_MESSAGE_LENGTH 256
#define MAX_URL_LENGTH 256
#define MAX_HEADER_LENGTH 50
#define MAX_FILENAME_LENGTH 256
#define MAX_LCD_MESSAGE_LENGTH 20

#define MAX_LENGTH_ID 3
#define MAX_FINGERPRINT 100

#define CONFIG_NAME "config.conf"

typedef enum {
    FAILED = 0,
    SUCCESS = 1
} Status_t;

#endif