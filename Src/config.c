#include "../Inc/config.h"

// Configuration variables for server settings, URLs, retries, and display messages
int g_server_port;
int g_month;
char g_url[MAX_URL_LENGTH];
char g_url_new_employee[MAX_URL_LENGTH];
char g_url_delete_employee[MAX_URL_LENGTH];
char g_url_check_delete[MAX_URL_LENGTH];
char g_header[MAX_HEADER_LENGTH];
int g_max_retries;
int g_db_sleep;
char g_lcd_message[MAX_LCD_MESSAGE_LENGTH];
char g_database_path[MAX_PATH_LENGTH];

/**
 * @brief Reads configuration data from a file and populates the provided config structure.
 *
 * This function opens the configuration file specified by `CONFIG_NAME` and reads various
 * configuration parameters from it. It populates the fields of the `Config_t` structure 
 * with values read from the file. In case of any errors during reading, it logs the error, 
 * closes the file, and returns a failure status.
 *
 * @param config Pointer to a `Config_t` structure that will be populated with configuration data.
 * 
 * @return Status of the configuration reading. Returns SUCCESS if all parameters are read
 *         successfully, otherwise returns FAILED.
 */
Status_t read_config(Config_t *config) 
{
    FILE *file = fopen(CONFIG_NAME, "r");
    if (!file) 
    {
        syslog_log(LOG_ERR, __func__, "strerror", "Could not open config file", strerror(errno));
        return FAILED;
    }
    // Read and process configuration data
    if (fscanf(file, "SERVER_PORT %d\n", &config->server_port) != SUCCESS)
    {
        syslog_log(LOG_ERR, __func__, "stderr", "Error reading SERVER_PORT from config file");
        fclose(file);
        return FAILED;
    }
    if (fscanf(file, "MONTH %d\n", &config->month) != SUCCESS)
    {
        syslog_log(LOG_ERR, __func__, "stderr", "Error reading MONTH from config file");
        fclose(file);
        return FAILED;
    }
    if (fscanf(file, "URL %s\n", config->url) != SUCCESS)
    {
        syslog_log(LOG_ERR, __func__, "stderr", "Error reading URL from config file");
        fclose(file);
        return FAILED;
    }
    if (fscanf(file, "URL_NEW_EMPLOYEE %s\n", config->url_new_employee) != SUCCESS)
    {
        syslog_log(LOG_ERR, __func__, "stderr", "Error reading URL_NEW_EMPLOYEE from config file");
        fclose(file);
        return FAILED;
    }
    if (fscanf(file, "URL_DELETE_EMPLOYEE %s\n", config->url_delete_employee) != SUCCESS)
    {
        syslog_log(LOG_ERR, __func__, "stderr", "Error reading URL_DELETE_EMPLOYEE from config file");
        fclose(file);
        return FAILED;
    }
    if (fscanf(file, "URL_CHECK_DELETE %s\n", config->url_check_delete) != SUCCESS)
    {
        syslog_log(LOG_ERR, __func__, "stderr", "Error reading URL_CHECK_DELETE from config file");
        fclose(file);
        return FAILED;
    }
    if (fscanf(file, "HEADER %[^\n]\n", config->header) != SUCCESS)
    {
        syslog_log(LOG_ERR, __func__, "stderr", "Error reading HEADER from config file");
        fclose(file);
        return FAILED;
    }
    if (fscanf(file, "MAX_RETRIES %d\n", &config->max_retries) != SUCCESS)
    {
        syslog_log(LOG_ERR, __func__, "stderr", "Error reading MAX_RETRIES from config file");
        fclose(file);
        return FAILED;
    }
    if (fscanf(file, "DATABASE_SLEEP_DURATION %d\n", &config->db_sleep) != SUCCESS)
    {
        syslog_log(LOG_ERR, __func__, "stderr", "Error reading DATABASE_SLEEP_DURATION from config file");
        fclose(file);
        return FAILED;
    }
    if (fscanf(file, "LCD_MESSAGE \"%[^\"]\"\n", config->lcd_message) != SUCCESS) 
    {
        syslog_log(LOG_ERR, __func__, "stderr", "Error reading LCD_MESSAGE from config file");
        fclose(file);
        return FAILED;
    }
    if (fscanf(file, "DATABASE_PATH %s\n", config->database_path) != SUCCESS) 
    {
        syslog_log(LOG_ERR, __func__, "stderr", "Error reading DATABASE_PATH from config file");
        fclose(file);
        return FAILED;
    }
    fclose(file);
    return SUCCESS;
}