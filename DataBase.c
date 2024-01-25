#include "database.h"

Database database;
// Global mutex for synchronizing database access
pthread_mutex_t databaseMutex = PTHREAD_MUTEX_INITIALIZER;

void DB_open() 
{
    const char* db_name = "employee.db";
    char* err_msg = 0;
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
            "Saved TEXT DEFAULT 'X');";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to create table: %s\n", err_msg);
        sqlite3_free(err_msg);
        exit(1);
    }
    // Initialize the mutex
    if (pthread_mutex_init(&databaseMutex, NULL) != 0) 
    {
        fprintf(stderr, "Failed to initialize mutex\n");
        exit(1);
    }
}

void DB_write(int ID, const char *date, const char *time, const char *direction)
{
    // Obtain the mutex before accessing the database
    if (pthread_mutex_lock(&databaseMutex) == 0)
    {
        sqlite3_stmt *stmt;
        // SQL query to insert data into the table
        const char *sql = "INSERT INTO employee_attendance (ID, Date, Time, Direction) VALUES (?, ?, ?, ?);";
        // Prepare the request
        if (sqlite3_prepare_v2(database->db, sql, -1, &stmt, NULL) != SQLITE_OK)
        {
            fprintf(stderr, "Failed to prepare request: %s\n", sqlite3_errmsg(database->db));
            exit(1);
        }
        // Binding values to request parameters
        sqlite3_bind_int(stmt, 1, ID);
        sqlite3_bind_text(stmt, 2, date, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, time, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, direction, -1, SQLITE_STATIC);
        // Execute the request
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            fprintf(stderr, "The request failed: %s\n", sqlite3_errmsg(database->db));
            exit(1);
        }
        // Finish the request
        sqlite3_finalize(stmt);
        // Release the mutex after performing operations
        pthread_mutex_unlock(&databaseMutex);
    }
}

void DB_close()
{
    // Obtain the mutex before accessing the database
    if (pthread_mutex_lock(&databaseMutex) == 0)
    {
        // Close the connection to the database
        sqlite3_close(database->db);
        // Release the mutex after performing operations
        pthread_mutex_unlock(&databaseMutex);
    }
}

void DB_find(int client_socket)
{
    // Obtain the mutex before accessing the database
    if (pthread_mutex_lock(&databaseMutex) == 0)
    {
        char *query = "SELECT * FROM employee_attendance WHERE Saved = 'X';";
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(database->db, query, -1, &stmt, NULL) != SQLITE_OK)
        {
            fprintf(stderr, "Failed to prepare request: %s\n", sqlite3_errmsg(database->db));
            return;
        }
        // Processing query results
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            char data[1024];
            int id = sqlite3_column_int(stmt, 0);
            const char *date = (const char *)sqlite3_column_text(stmt, 1);
            const char *time = (const char *)sqlite3_column_text(stmt, 2);
            const char *direction = (const char *)sqlite3_column_text(stmt, 3);

            snprintf(data, sizeof(data), "%d,%s,%s,%s",
                     id,
                     date ? date : "",            // Date
                     time ? time : "",            // Time
                     direction ? direction : ""); // Direction

            if (sendData(client_socket, data))
            {
                printf("Data sent to the server: %s\n", data);
                DB_update(id);
            }
        }
        // Finish the request
        sqlite3_finalize(stmt);
        pthread_mutex_unlock(&databaseMutex);
    }
}
void DB_update(int id)
{

    char updateQuery[256];
    snprintf(updateQuery, sizeof(updateQuery), "UPDATE employee_attendance SET Saved = 'V' WHERE ID = %d;", id);
    char *err_msg = 0;
    int rc = sqlite3_exec(database->db, updateQuery, 0, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to update data in the database: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    else
        printf("Data in the database updated: Set 'Saved' to 'V' for ID=%d\n", id);
}