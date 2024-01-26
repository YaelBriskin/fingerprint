#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "client_socket.h"


void DB_open();
void DB_write(int ID, const char* date, const char* time, const char* direction);
void DB_close();
void DB_find();
void DB_update(int id);

#endif  // DATABASE_H