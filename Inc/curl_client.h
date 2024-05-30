#ifndef CURL_CLIENT_H
#define CURL_CLIENT_H

#include "syslog_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <pthread.h>
#include "defines.h"
#include "config.h"

int send_request(const char *post_data, const char *URL);
Status_t send_json_data (int tz, const char* event, int timestamp, const char* fpm);
Status_t send_json_new_employee (int id, int timestamp);
#endif 