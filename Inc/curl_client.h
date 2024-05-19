#ifndef CURL_CLIENT_H
#define CURL_CLIENT_H

#include "syslog_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#define URL "https://st-portal.online/RTG-Reports/api/attendance/fingerprint"

int send_request(const char *post_data);
int send_json_data (int tz, const char* event, int timestamp, const char* fpm);
int send_json_new_employee (int id);
#endif 