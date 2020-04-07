#include "database.h"

Database DB_open() 
{
    const char* db_name = "employee.db";
    char* err_msg = 0;
    Database database;
    // Open a connection to the database (if there is no database, it will be created automatically)    
    int rc = sqlite3_open(db_name, &(database.db));

    if (rc != SQLITE_OK) 
    {
        fprintf(stderr, "Failed to open/create database: %s\n", sqlite3_errmsg(database.db));
        exit(1);
    }
    char *sql = "CREATE TABLE IF NOT EXISTS employee_attendance ("
            "ID INTEGER PRIMARY KEY ,"
            "Date TEXT,"
            "Time TEXT,"
            "Direction TEXT,"
            "Saved TEXT);";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to create table: %s\n", err_msg);
        sqlite3_free(err_msg);
        exit(1);
    }

    return database;
}

void DB_write(Database* database, const char* date, const char* time, const char* direction, int saved) 
{
    char sql[200];
    sqlite3_stmt* stmt;

    // SQL query to insert data into the table  
    sprintf(sql, "INSERT INTO employee_attendance (Date, Time, Direction, Saved) VALUES ('%s', '%s', '%s', %d);",
            date, time, direction, saved);

    // Prepare the request
    int rc = sqlite3_prepare_v2(database->db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare request: %s\n", sqlite3_errmsg(database->db));
        exit(1);
    }
    // Execute the request
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "The request failed: %s\n", sqlite3_errmsg(database->db));
        exit(1);
    }
    // Finish the request
    sqlite3_finalize(stmt);
}

void DB_close(Database* database) 
{
    // Close the connection to the database
    sqlite3_close(database->db);
}

void DB_find(Database* database, int client_socket)
{
    char* query = "SELECT * FROM employee_attendance WHERE Saved = 'X';";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(database->db, query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) 
    {
        fprintf(stderr, "Failed to prepare request: %s\n", sqlite3_errmsg(database->db));
        return;
    }
    // Processing query results
    while (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        char data[1024];
        int id = sqlite3_column_int(stmt, 0);
        snprintf(data, sizeof(data), "%d,%s,%s,%s",
                 id,  
                 sqlite3_column_text(stmt, 1),  //Date
                 sqlite3_column_text(stmt, 2),  //Time
                 sqlite3_column_text(stmt, 3)); //Direction
        if(sendData(client_socket, data))
        {
            printf("Data sent to the server: %s\n", data);
            DB_update(database, id);
        }
    }
    // Finish the request
    sqlite3_finalize(stmt);
}
void DB_update(Database* database, int id)
{
    char updateQuery[256];
    snprintf(updateQuery, sizeof(updateQuery), "UPDATE employee_attendance SET Saved = 'V' WHERE ID = %d;", id);
    char* err_msg = 0;
    int rc = sqlite3_exec(database->db, updateQuery, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to update data in the database: %s\n", err_msg);
        sqlite3_free(err_msg);
    } else {
        printf("Data in the database updated: Set 'Saved' to 'V' for ID=%d\n", id);
    }
}