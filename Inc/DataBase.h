#ifndef DATABASE_H
#define DATABASE_H

#define _GNU_SOURCE

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "defines.h"
#include "curl_client.h"
#include "syslog_util.h"
#include "config.h"

void DB_open();
void DB_newEmployee();
Status_t DB_write(int ID, int Timestamp, const char *direction,const char *FPM);
void DB_close();
int DB_find();
void DB_update(int id);
Status_t DB_delete(int ID);
void DB_delete_old_records(time_t lastDay);
int getNextAvailableID();
int DB_check_id_exists(int id);
int DB_restore(int id);
int DB_find_ID(int id_to_check);
#endif  // DATABASE_H