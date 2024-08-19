#include "../Inc/curl_client.h"

pthread_mutex_t httpMutex = PTHREAD_MUTEX_INITIALIZER;

// Callback function for writing data to a file
size_t PostWriteCallback(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}
// Callback function to write data to string
size_t GetWriteCallback(void *ptr, size_t size, size_t nmemb, struct StringBuffer *strBuf)
{
    size_t new_data_size = size * nmemb;
    char *new_buffer = realloc(strBuf->buffer, strBuf->size + new_data_size + 1);
    if (new_buffer == NULL)
    {
        return 0;
    }

    strBuf->buffer = new_buffer;
    memcpy(strBuf->buffer + strBuf->size, ptr, new_data_size);
    strBuf->size += new_data_size;
    strBuf->buffer[strBuf->size] = '\0';

    return new_data_size;
}
/**
 * @brief Sends an HTTP POST request with the given data.
 *
 * This function initializes the cURL library, sets up the HTTP POST request with the provided
 * data, and sends it to the predefined URL. It checks the response code and logs any errors.
 *
 * @param post_data The JSON data to send in the POST request.
 * @param URL The URL to which the request will be sent.
 * @return 1 if the request was successful, 0 otherwise.
 */
int send_post_request(const char *post_data, const char *URL)
{
    CURL *curl;
    CURLcode res;
    int result = SUCCESS;
    long response_code;

    if (pthread_mutex_lock(&httpMutex) != MUTEX_OK)
    {
        // Handle mutex acquisition error
        writeToFile(file_URL, __func__, "Failed to lock mutex");
        return FAILED;
    }
    curl = curl_easy_init();
    if (curl)
    {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, g_header);

        if (headers == NULL)
        {
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            writeToFile(file_URL, __func__, "Failed to set HTTP headers");
            pthread_mutex_unlock(&httpMutex);
            return FAILED;
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        // Set the URL to which the request will be sent
        curl_easy_setopt(curl, CURLOPT_URL, URL);
        // Setting the request method (POST)
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        // Setting the data to be sent
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

        // Add a callback function to record response data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, PostWriteCallback);
        // Specify the file where the response data will be written
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file_URL);

        // Execute the request
        res = curl_easy_perform(curl);
        // Check the server response status code
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        // Check the success of the request
        if (res != CURLE_OK)
        {
            char log_message[MAX_LOG_MESSAGE_LENGTH];
            snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "curl_easy_perform() failed . ERROR: %s ", curl_easy_strerror(res));
            writeToFile(file_URL, __func__, log_message);
            result = FAILED;
        }
        else if (response_code >= 400)
        {
            char log_message[MAX_LOG_MESSAGE_LENGTH];
            snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "HTTP request failed with response code: %ld", response_code);
            writeToFile(file_URL, __func__, log_message);
            result = FAILED;
        }
        // Release resources
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    else
        result = FAILED;

    pthread_mutex_unlock(&httpMutex);
    return result;
}
/**
 * @brief Sends an HTTP GET request to the given URL.
 *
 * This function initializes the cURL library, sets up the HTTP GET request, and sends it to the predefined URL.
 * It checks the response code and logs any errors.
 *
 * @param URL The URL to which the request will be sent.
 * @return 1 if the request was successful, 0 otherwise.
 */
int send_get_request(const char *URL)
{
    CURL *curl;
    CURLcode res;
    long response_code;

    struct StringBuffer response = {.buffer = NULL, .size = 0};

    if (pthread_mutex_lock(&httpMutex) != MUTEX_OK)
    {
        // Handle mutex acquisition error
        writeToFile(file_URL, __func__, "Failed to lock mutex");
        return FAILED;
    }

    curl = curl_easy_init();
    if (curl)
    {
        // Set the URL to which the request will be sent
        curl_easy_setopt(curl, CURLOPT_URL, URL);
        // Setting the request method (GET)
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

        // Add a callback function to record response data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, GetWriteCallback);
        // Specify the buffer where the response data will be written
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Execute the request
        res = curl_easy_perform(curl);
        // Check the server response status code
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        // Check the success of the request
        if (res != CURLE_OK)
        {
            char log_message[MAX_LOG_MESSAGE_LENGTH];
            snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "curl_easy_perform() failed. ERROR: %s", curl_easy_strerror(res));
            writeToFile(file_URL, __func__, log_message);
            curl_easy_cleanup(curl);
            pthread_mutex_unlock(&httpMutex);
            return FAILED;
        }
        else if (response_code >= 400)
        {
            char log_message[MAX_LOG_MESSAGE_LENGTH];
            snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "HTTP request failed with response code: %ld", response_code);
            writeToFile(file_URL, __func__, log_message);
            curl_easy_cleanup(curl);
            pthread_mutex_unlock(&httpMutex);
            return FAILED;
        }

        // Process the response data
        if (response.size > 0)
        {
            if (process_response(response.buffer) != SUCCESS)
            {
                curl_easy_cleanup(curl);
                pthread_mutex_unlock(&httpMutex);
                return FAILED;
            }
        }
        // Release resources
        curl_easy_cleanup(curl);
    }
    else
    {
        pthread_mutex_unlock(&httpMutex);
        return FAILED;
    }

    // Free the allocated buffer
    if (response.buffer != NULL)
    {
        free(response.buffer);
    }
    pthread_mutex_unlock(&httpMutex);
    return SUCCESS;
}

/**
 * @brief Sends an HTTP DELETE request to the given URL with the specified data.
 *
 * This function initializes the cURL library, sets up the HTTP DELETE request with the provided
 * data, and sends it to the predefined URL. It checks the response code and logs any errors.
 *
 * @param URL The URL to which the request will be sent.
 * @param data The JSON data to send in the DELETE request.
 * @return 1 if the request was successful, 0 otherwise.
 */
int send_delete_request(const char *URL, const char *data)
{
    CURL *curl;
    CURLcode res;
    int result = SUCCESS;
    long response_code;

    if (pthread_mutex_lock(&httpMutex) != MUTEX_OK)
    {
        // Handle mutex acquisition error
        writeToFile(file_URL, __func__, "Failed to lock mutex");
        return FAILED;
    }
    curl = curl_easy_init();
    if (curl)
    {
        // Set the URL to which the request will be sent
        curl_easy_setopt(curl, CURLOPT_URL, URL);
        // Setting the request method (DELETE)
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

        // Add a callback function to record response data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, PostWriteCallback);
        // Specify the file where the response data will be written
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file_URL);

        // Set the data to be sent
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

        // Execute the request
        res = curl_easy_perform(curl);
        // Check the server response status code
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        // Check the success of the request
        if (res != CURLE_OK)
        {
            char log_message[MAX_LOG_MESSAGE_LENGTH];
            snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "curl_easy_perform() failed. ERROR: %s", curl_easy_strerror(res));
            writeToFile(file_URL, __func__, log_message);
            result = FAILED;
        }
        else if (response_code >= 400)
        {
            char log_message[MAX_LOG_MESSAGE_LENGTH];
            snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "HTTP request failed with response code: %ld", response_code);
            writeToFile(file_URL, __func__, log_message);
            result = FAILED;
        }
        // Release resources
        curl_easy_cleanup(curl);
    }
    else
    {
        result = FAILED;
    }

    pthread_mutex_unlock(&httpMutex);
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
Status_t send_json_data(int id, const char *event, int timestamp, const char *fpm)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddStringToObject(root, "event", event);
    cJSON_AddNumberToObject(root, "timestamp", timestamp);
    cJSON_AddStringToObject(root, "fpm", fpm);

    char *json_data = cJSON_Print(root);
    if (json_data == NULL)
    {
        cJSON_Delete(root);
        return FAILED;
    }
    int result = send_post_request(json_data, g_url);
    cJSON_Delete(root);
    free(json_data);

    if (result)
    {
        return SUCCESS;
    }
    return FAILED;
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
Status_t send_json_new_employee(int id, int timestamp)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddNumberToObject(root, "timestamp", timestamp);

    // if 'V' it means the employee registered using the fingerprint module if 'X' means using the keypad
    char *json_data = cJSON_Print(root);
    if (json_data == NULL)
    {
        cJSON_Delete(root);
        return FAILED;
    }
    int result = send_post_request(json_data, g_url_new_employee);
    cJSON_Delete(root);
    free(json_data);
    if (result != SUCCESS)
    {
        char log_message[MAX_LOG_MESSAGE_LENGTH];
        snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Failed to send request. ERROR: ", strerror(errno));
        writeToFile(file_URL, __func__, log_message);
        return FAILED;
    }
    return SUCCESS;
}

/**
 * @brief Sends confirmation of employee deletion to the server.
 *
 * @param id The ID of the employee who was deleted.
 * @return 1 if the request was successful, 0 otherwise.
 */
Status_t send_json_ack_delete(int id)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "id", id);
    char *json_data = cJSON_Print(root);
    if (json_data == NULL)
    {
        cJSON_Delete(root);
        return FAILED;
    }

    // Append ":id" to the g_url_check_delete
    char url_with_id[MAX_URL_LENGTH];
    snprintf(url_with_id, sizeof(url_with_id), "%s/%d", g_url_check_delete, id);

    int result = send_delete_request(url_with_id, json_data);
    if (result != SUCCESS)
    {
        char log_message[MAX_LOG_MESSAGE_LENGTH];
        snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Failed to send acknolage request for deletions. Error: %s", strerror(errno));
        writeToFile(file_URL, __func__, log_message);
        free(json_data);
        return FAILED;
    }
    free(json_data);
    return SUCCESS;
}

/**
 * @brief Processes the server response.
 *
 * @param response JSON string representing the server response.
 */
int process_response(const char *response)
{
    // Parse JSON response
    cJSON *json = cJSON_Parse(response);
    if (json == NULL)
    {
        char log_message[MAX_LOG_MESSAGE_LENGTH];
        snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Failed to send request for deletions. Error: %s", strerror(errno));
        writeToFile(file_URL, __func__, log_message);
        return FAILED;
    }
    // Check if the response is an array
    if (cJSON_IsArray(json))
    {
        int id_count = cJSON_GetArraySize(json);
        for (int i = 0; i < id_count; ++i)
        {
            cJSON *id_item = cJSON_GetArrayItem(json, i);
            // Check if each element is a number
            if (cJSON_IsNumber(id_item))
            {
                int id_to_delete = id_item->valueint;
                if (DB_delete(id_to_delete) == SUCCESS && deleteModel((uint16_t)id_to_delete) == SUCCESS)
                {
                    if (send_json_ack_delete(id_to_delete) == FAILED)
                    {
                        char log_message[MAX_LOG_MESSAGE_LENGTH];
                        snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Failed to send acknowledgment for deletion of employee with ID: %d", id_to_delete);
                        writeToFile(file_URL, __func__, log_message);
                        cJSON_Delete(json);
                        return FAILED;
                    }
                    char log_message[MAX_LOG_MESSAGE_LENGTH];
                    snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Successfully deleted employee with ID: %d\n", id_to_delete);
                    writeToFile(file_URL, __func__, log_message);
                }
                else
                {
                    char log_message[MAX_LOG_MESSAGE_LENGTH];
                    snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Failed to delete employee with ID: %d\n", id_to_delete);
                    writeToFile(file_URL, __func__, log_message);
                }
            }
            else
            {
                char log_message[MAX_LOG_MESSAGE_LENGTH];
                snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Invalid ID format in response. Error: %s", strerror(errno));
                writeToFile(file_URL, __func__, log_message);
            }
        }
    }
    else if (cJSON_IsNull(json))
    {
        writeToFile(file_URL, __func__, "No IDs to delete in the response");
    }
    else
    {
        char log_message[MAX_LOG_MESSAGE_LENGTH];
        snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Invalid JSON format: expected an array. Error: %s", strerror(errno));
        writeToFile(file_URL, __func__, log_message);
        cJSON_Delete(json);
        return FAILED;
    }
    // Clean up the JSON data structure
    cJSON_Delete(json);
    return SUCCESS;

}
