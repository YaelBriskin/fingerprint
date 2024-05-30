#ifndef DEFINES_H
#define DEFINES_H

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
#define SLEEP_LCD 2
#define SLEEP_DURATION 3
#define DATABASE_SLEEP_DURATION 120
#define TRUE "true"
#define FALSE "false"
#define IN "in"
#define OUT "out"
#define HELLO "Hello"
#define GOODBYE "Goodbye"
#define DELAY 10000
#define TIME_STR_LEN 20

#define MESSAGE_LEN 50

typedef enum {
    FAILED = 0,
    SUCCESS = 1
} Status_t;

#endif