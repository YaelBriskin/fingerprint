#include "../Inc/curl_client.h"

/**
 * @brief Sends an HTTP POST request with the given data.
 *
 * This function initializes the cURL library, sets up the HTTP POST request with the provided
 * data, and sends it to the predefined URL. It checks the response code and logs any errors.
 *
 * @param post_data The JSON data to send in the POST request.
 * @return 1 if the request was successful, 0 otherwise.
 */
int send_request(const char *post_data) 
{
    CURL *curl;
    CURLcode res;
    int result = 1;
    long response_code;

    // Initialize the cURL library
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
        // Check the success of the request
        if (res != CURLE_OK) 
        {
            syslog_log(LOG_ERR, __func__, "format", "curl_easy_perform() failed: %s", curl_easy_strerror(res));
            result = 0; 
        }

        // Release resources
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    } 
    else 
        result = 0; 
    curl_global_cleanup();
    return result;
}
/**
 * @brief Sends JSON data representing an event to the server.
 *
 * This function creates a JSON object with the given parameters and sends it to the server.
 *
 * @param id The ID of the employee.
 * @param event The event type (e.g., "IN" or "OUT").
 * @param timestamp The timestamp of the event.
 * @param fpm The fingerprint module identifier.
 * @return 1 if the data was successfully sent, 0 otherwise.
 */
int send_json_data (int id, const char* event, int timestamp, const char* fpm)
{   
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddStringToObject(root, "event",event);
    cJSON_AddNumberToObject(root, "timestamp", timestamp);
    cJSON_AddStringToObject(root, "fpm", fpm);  

    char *json_data = cJSON_Print(root);
    int result = send_request(json_data);
    cJSON_Delete(root);
    free(json_data);

    if (result)
        return 1;
    return 0;
}

/**
 * @brief Sends JSON data representing a new employee to the server.
 *
 * This function creates a JSON object with the given parameters and sends it to the server.
 *
 * @param id The ID of the new employee.
 * @param timestamp The timestamp of the registration.
 * @return 1 if the data was successfully sent, 0 otherwise.
 */
int send_json_new_employee (int id,int timestamp)
{   
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddNumberToObject(root, "timestamp", timestamp);

    //if 'V' it means the employee registered using the fingerprint module if 'X' means using the keypad
    char *json_data = cJSON_Print(root);
    //printf("json_data= %s",json_data);
    int result = send_request(json_data);
    cJSON_Delete(root);
    free(json_data);
    if (result != 0)
    {
        //fprintf(stderr, "Failed to send request.\n");
        return 0;
    }
    return 1;
}