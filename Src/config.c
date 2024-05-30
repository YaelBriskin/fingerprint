#include "../Inc/config.h"


int g_server_port;
int g_month;
char g_url[MAX_URL_LENGTH];
char g_url_new_employee[MAX_URL_LENGTH];
char g_header[MAX_HEADER_LENGTH];
char g_header_new_employee[MAX_HEADER_LENGTH];
int g_max_retries;

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
        syslog_log(LOG_ERR, __func__, "stderr", "Error reading URL from config file");
        fclose(file);
        return FAILED;
    }
    if (fscanf(file, "HEADER %[^\n]\n", config->header) != SUCCESS)
    {
        syslog_log(LOG_ERR, __func__, "stderr", "Error reading HEADER from config file");
        fclose(file);
        return FAILED;
    }
        if (fscanf(file, "HEADER_NEW_EMPLOYEE %[^\n]\n", config->header_new_employee) != SUCCESS)
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

    fclose(file);
    return SUCCESS;
}