#ifndef CURL_CLIENT_H
#define CURL_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include "defines.h"
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <pthread.h>
#include "file_utils.h"
#include "config.h"
#include "DataBase.h"
#include "FP_delete.h"

struct StringBuffer
{
    char *buffer;
    size_t size;
};

size_t PostWriteCallback(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t GetWriteCallback(void *ptr, size_t size, size_t nmemb, struct StringBuffer *strBuf);
int send_post_request(const char *post_data, const char *URL);
int send_get_request(const char *URL);
int send_delete_request(const char *URL, const char *data);
Status_t send_json_data (int tz, const char* event, int timestamp, const char* fpm);
Status_t send_json_new_employee (int id, int timestamp);
Status_t send_json_ack_delete(int id);
int process_response(const char *response);

#endif 