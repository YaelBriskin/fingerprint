#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../Inc/client_socket_http.h"
#include "../Inc/curl_client.h"


void DB_open();
void DB_write(int ID, const char *date, const char *time, const char *direction,const char *FPM);
void DB_close();
void DB_find();
void DB_update(int id);
int getNextAvailableID();

#endif  // DATABASE_H