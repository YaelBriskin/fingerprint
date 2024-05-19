#include "../Inc/curl_client.h"

int send_request(const char *post_data) 
{
    printf("%s\r\n",post_data);
    CURL *curl;
    CURLcode res;
    int result = 0;
    long response_code;
    // Инициализация библиотеки libcurl
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) 
    {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "device-id: 1"); 
        // Set the URL to which the request will be sent
        curl_easy_setopt(curl, CURLOPT_URL, URL);
        // Setting the request method (POST)
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        // Setting the data to be sent
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        // Execute the request
        res = curl_easy_perform(curl);
        // Check the server response status code
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        printf("\r\nResponse code: %ld\n", response_code);
        // Check the success of the request
        if (res != CURLE_OK) 
        {
            syslog_log(LOG_ERR, __func__, "format", "curl_easy_perform() failed: %s", curl_easy_strerror(res));
            //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            result = 1; 
        }

        // Release resources
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    } 
    else 
        result = 1; 
    curl_global_cleanup();
    return result;
}

int send_json_data (int id, const char* event, int timestamp, const char* fpm)
{   
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "tz", id);
    cJSON_AddStringToObject(root, "event",event); //"out" or "in" 
    cJSON_AddNumberToObject(root, "timestamp", timestamp);
    cJSON_AddStringToObject(root, "fpm", fpm);  //'X' or 'V' 
    //if 'V' it means the employee registered using the fingerprint module if 'X' means using the keypad
    char *json_data = cJSON_Print(root);
    printf("json_data= %s",json_data);
    int result = send_request(json_data);
    cJSON_Delete(root);
    free(json_data);

    if (result != 0)
    {
        fprintf(stderr, "Failed to send request.\n");
        return 0;
    }
    return 1;
}
int send_json_new_employee (int id)
{   
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "tz", id);
    //if 'V' it means the employee registered using the fingerprint module if 'X' means using the keypad
    char *json_data = cJSON_Print(root);
    printf("json_data= %s",json_data);
    int result = send_request(json_data);
    cJSON_Delete(root);
    free(json_data);
    if (result != 0)
    {
        fprintf(stderr, "Failed to send request.\n");
        return 0;
    }
    return 1;
}