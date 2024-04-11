#include "../Inc/curl_client.h"

int send_request(const char *post_data) 
{
    CURL *curl;
    CURLcode res;
    int result = 0;
    long response_code;
    // Инициализация библиотеки libcurl
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) 
    {
        // Set the URL to which the request will be sent
        curl_easy_setopt(curl, CURLOPT_URL, URL);
        // Setting the request method (POST)
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        // Setting the data to be sent
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
        // Execute the request
        res = curl_easy_perform(curl);
        // Check the server response status code
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        printf("Response code: %ld\n", response_code);
        // Check the success of the request
        if (res != CURLE_OK) 
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            result = 1; 
        }
        else
            printf("Data successfully sent to the server!\n");
        // Release resources
        curl_easy_cleanup(curl);
    } 
    else 
        result = 1; 
    curl_global_cleanup();
    return result;
}
