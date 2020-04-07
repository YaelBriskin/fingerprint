#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    sqlite3* db;
} Database;

Database DB_open();
void DB_write(Database* database, const char* date, const char* time, const char* direction, int saved);
void DB_close(Database* database);
void DB_find(Database* database, int client_socket);

#endif  // DATABASE_H