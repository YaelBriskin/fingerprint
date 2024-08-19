#ifndef READ_CONF_FILE_H
#define READ_CONF_FILE_H

#include "defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "syslog_util.h"

typedef struct 
{
    int server_port;
    int month;
    char url[MAX_URL_LENGTH];
    char url_new_employee[MAX_URL_LENGTH];
    char url_delete_employee[MAX_URL_LENGTH];
    char url_check_delete[MAX_URL_LENGTH];
    char header[MAX_HEADER_LENGTH];
    int max_retries;
    int db_sleep;
    char lcd_message[MAX_LCD_MESSAGE_LENGTH];
} Config_t;

// Declare global variables
extern int g_server_port;
extern int g_month;
extern char g_url[MAX_URL_LENGTH];
extern char g_url_new_employee[MAX_URL_LENGTH];
extern char g_url_delete_employee[MAX_URL_LENGTH];
extern char g_url_check_delete[MAX_URL_LENGTH];
extern char g_header[MAX_HEADER_LENGTH];
extern int g_max_retries;
extern int g_db_sleep;
extern char g_lcd_message[MAX_LCD_MESSAGE_LENGTH];

Status_t read_config(Config_t *config);

#endif 