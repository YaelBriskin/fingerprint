#ifndef DATABASE_H
#define DATABASE_H

#define _GNU_SOURCE

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../Inc/curl_client.h"
#include "../Inc/syslog_util.h"

void DB_open();
void DB_newEmployee();
int DB_write(int ID, int Timestamp, const char *direction,const char *FPM);
void DB_close();
void DB_find();
void DB_update(int id);
void DB_delete(int ID);
int getNextAvailableID();

#endif  // DATABASE_H