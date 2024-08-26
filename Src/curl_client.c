#include "../Inc/curl_client.h"

pthread_mutex_t httpMutex = PTHREAD_MUTEX_INITIALIZER;

struct StringBuffer
{
    char *buffer;
    size_t size;
};
/**
 * @brief Callback function for writing POST data to a file.
 *
 * This function is used by cURL to write the response data to a file. It writes the data received from
 * the POST request into the file specified by the `stream` parameter.
 *
 * @param ptr Pointer to the data to be written.
 * @param size Size of each data element.
 * @param nmemb Number of data elements.
 * @param stream File pointer to write data to.
 * @return The number of bytes written.
 */
size_t PostWriteCallback(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    if (written != size * nmemb)
    {
        writeToFile(file_URL, __func__, "Failed to write complete data to file");
    }
    return written;
}

/**
 * @brief Callback function to write data to a string buffer.
 *
 * This function is used by cURL to write the response data to a dynamically allocated string buffer.
 * It appends the received data to the buffer and adjusts the buffer size accordingly.
 *
 * @param ptr Pointer to the data to be written.
 * @param size Size of each data element.
 * @param nmemb Number of data elements.
 * @param strBuf Pointer to the StringBuffer structure where data will be stored.
 * @return The number of bytes written.
 */
size_t GetWriteCallback(void *ptr, size_t size, size_t nmemb, void *userp)
{
    size_t new_data_size = size * nmemb;
    struct StringBuffer *strBuf = (struct StringBuffer *)userp;

    // Reallocate memory for the buffer
    char *new_buffer = realloc(strBuf->buffer, strBuf->size + new_data_size + 1);
    if (new_buffer == NULL)
    {
        writeToFile(file_URL, __func__, "Failed to allocate memory for response buffer");
        free(strBuf->buffer);
        return 0;
    }

    strBuf->buffer = new_buffer;

    // Copy the new data to the buffer
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
    if (!curl)
    {
        pthread_mutex_unlock(&httpMutex);
        return FAILED;
    }
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, g_header);

    if (headers == NULL)
    {
        curl_easy_cleanup(curl);
        writeToFile(file_URL, __func__, "Failed to set HTTP headers");
        pthread_mutex_unlock(&httpMutex);
        return FAILED;
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    // Set the URL to which the request will be sent
    curl_easy_setopt(curl, CURLOPT_URL, URL);
    //  Setting the request method (GET)
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
    else if (response_code != 200)
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
        pthread_mutex_unlock(&httpMutex); // Release mutex before processing response
        if (process_response(response.buffer) != SUCCESS)
        {
            curl_easy_cleanup(curl);
            return FAILED;
        }
    }
    // Release resources
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

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
    printf("send_delete_request\n");
    CURL *curl;
    CURLcode res;
    int result = SUCCESS;
    long response_code;

    if (pthread_mutex_lock(&httpMutex) != MUTEX_OK)
    {
        // Handle mutex acquisition error
        writeToFile(file_URL, __func__, "Failed to lock mutex");
        printf("Failed to lock mutex\n");
        return FAILED;
    }
    printf("before curl_easy_init\n");
    curl = curl_easy_init();
    printf("after curl_easy_init\n");
    if (curl)
    {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, g_header);

        if (headers == NULL)
        {
            curl_easy_cleanup(curl);
            writeToFile(file_URL, __func__, "Failed to set HTTP headers");
            printf("Failed to set HTTP headers\n");
            pthread_mutex_unlock(&httpMutex);
            return FAILED;
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
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
            printf("%s\n",log_message);
            result = FAILED;
        }
        else if (response_code >= 400)
        {
            char log_message[MAX_LOG_MESSAGE_LENGTH];
            snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "HTTP request failed with response code: %ld", response_code);
            writeToFile(file_URL, __func__, log_message);
            printf("%s\n",log_message);
            result = FAILED;
        }
        // Release resources
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    else
    {
        result = FAILED;
        printf("curl_easy_init error\n");
    }

    printf("finish\n");
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
    printf("send_json_ack_delete\n");
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
    printf("process_response\n");
    int success = SUCCESS;
    char log_message[MAX_LOG_MESSAGE_LENGTH];
    // Parse JSON response
    cJSON *json = cJSON_Parse(response);
    if (json == NULL)
    {
        writeToFile(file_URL, __func__, "Failed to send request for deletions");
        return FAILED;
    }

    // Check if the response is an array
    if (!cJSON_IsArray(json))
    {
        snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Invalid JSON format: expected an array. Error: %s", cJSON_GetErrorPtr());
        writeToFile(file_URL, __func__, log_message);
        cJSON_Delete(json);
        return FAILED;
    }
    // Getting the number of elements to remove from JSON-array
    int id_count = cJSON_GetArraySize(json);
    if (id_count == 0)
    {
        writeToFile(file_URL, __func__, "No IDs to delete in the response");
    }
    for (int i = 0; i < id_count; ++i)
    {
        cJSON *id_item = cJSON_GetArrayItem(json, i);
        // Check if each element is a number
        if (!cJSON_IsNumber(id_item))
        {
            snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Invalid ID format in response. Error: %s", cJSON_GetErrorPtr());
            writeToFile(file_URL, __func__, log_message);
            // Continue to the next ID even if one is invalid
            continue;
        }

        int id_to_delete = id_item->valueint;
        // Check if the ID exists in the database
        if (DB_check_id_exists(id_to_delete) == SUCCESS)
        {
            // Attempt to delete from the database
            int db_result = DB_delete(id_to_delete);
            if (db_result == FAILED)
            {
                snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Failed to delete employee with ID: %d from the database", id_to_delete);
                writeToFile(file_URL, __func__, log_message);
                printf("%s\n",log_message);
                // Skip to the next ID if database deletion failed
                continue;
            }
            // Attempt to delete from the fingerprint module
            int model_result = deleteModel((uint16_t)id_to_delete);
            if (model_result != SUCCESS)
            {
                snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Failed to delete employee with ID: %d from the fingerprint module", id_to_delete);
                writeToFile(file_URL, __func__, log_message);
                printf("%s\n",log_message);
                // Restore the record in the database if deletion from module failed
                if (DB_restore(id_to_delete) == FAILED)
                {
                    snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Failed to restore employee with ID: %d in the database", id_to_delete);
                    writeToFile(file_URL, __func__, log_message);
                    printf("%s\n",log_message);
                }
                continue; // Skip to the next ID
            }
            // Send acknowledgment for deletion
            if (send_json_ack_delete(id_to_delete) == FAILED)
            {
                snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Failed to send acknowledgment for deletion of employee with ID: %d", id_to_delete);
                writeToFile(file_URL, __func__, log_message);
                printf("%s\n",log_message);
                cJSON_Delete(json);
                return FAILED;
            }
            else
            {
                snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Successfully deleted employee with ID: %d\n", id_to_delete);
                writeToFile(file_URL, __func__, log_message);
                printf("%s\n",log_message);
            }
        }
        else
        {
            // If ID does not exist in the database, send acknowledgment
            if (send_json_ack_delete(id_to_delete) == FAILED)
            {
                snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "Failed to send acknowledgment for deletion of employee with ID: %d", id_to_delete);
                writeToFile(file_URL, __func__, log_message);
                cJSON_Delete(json);
                printf("%s\n",log_message);
                return FAILED;
            }
            snprintf(log_message, MAX_LOG_MESSAGE_LENGTH, "ID %d does not exist in the database.ID %d was removed from the server only.", id_to_delete, id_to_delete);
            writeToFile(file_URL, __func__, log_message);
            printf("%s\n",log_message);
        }
    }
    // Clean up the JSON data structure
    cJSON_Delete(json);
    return SUCCESS;
}
