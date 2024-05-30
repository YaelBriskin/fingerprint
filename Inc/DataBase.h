#ifndef DATABASE_H
#define DATABASE_H

#define _GNU_SOURCE

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../Inc/curl_client.h"
#include "../Inc/syslog_util.h"
#include "../Inc/defines.h"
#include "config.h"

#define MONTH 2

void DB_open();
void DB_newEmployee();
Status_t DB_write(int ID, int Timestamp, const char *direction,const char *FPM);
void DB_close();
int DB_find();
void DB_update(int id);
void DB_delete(int ID);
void DB_delete_old_records(time_t lastDay);
int getNextAvailableID();

#endif  // DATABASE_H