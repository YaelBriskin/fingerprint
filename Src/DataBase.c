#include "../Inc/DataBase.h"

sqlite3 *db_attendance;
pthread_mutex_t databaseMutex;

int getNextAvailableID() 
{
    int id = 0;
    char *query = "SELECT IDENT_CURRENT ('attendance')";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db_attendance, query, -1, &stmt, NULL) != SQLITE_OK) 
    {
        printf("Failed to execute query: %s\n", sqlite3_errmsg(db_attendance));
        return id;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) 
        id = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return id + 1;
}

void DB_open()
{
    const char *db_name = "employee_attendance.db";
    char *err_msg = 0;

    // Open a connection to the "attendance" database
    int rc2 = sqlite3_open(db_name, &db_attendance);
    if (rc2 != SQLITE_OK)
    {
        fprintf(stderr, "Failed to open attendance database: %s\n", sqlite3_errmsg(db_attendance));
        exit(1);
    }

    // Open a connection to the database (if there is no database, it will be created automatically)
    char *sql = "CREATE TABLE IF NOT EXISTS attendance ("
                "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                "Date DATE NOT NULL,"
                "Time TIME NOT NULL,"
                "Direction TEXT NOT NULL,"
                "Saved TEXT DEFAULT 'X' NOT NULL,"
                "FPM TEXT NOT NULL,"
                ");";

    rc2 = sqlite3_exec(db_attendance, sql, 0, 0, &err_msg);
    if (rc2 != SQLITE_OK)
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

void DB_write(int ID, const char *date, const char *time, const char *direction,const char *FPM)
{
    // Obtain the mutex before accessing the database
    if (pthread_mutex_lock(&databaseMutex) == 0)
    {
        sqlite3_stmt *stmt;
        // SQL query to insert data into the table
        const char *sql = "INSERT INTO attendance (ID, Date, Time, Direction, FPM) VALUES (?, ?, ?, ?, ?);";
        // Prepare the request
        if (sqlite3_prepare_v2(db_attendance, sql, -1, &stmt, NULL) != SQLITE_OK)
        {
            fprintf(stderr, "Failed to prepare request: %s\n", sqlite3_errmsg(db_attendance));
            exit(1);
        }
        // Binding values to request parameters
        sqlite3_bind_int(stmt, 1, ID);
        sqlite3_bind_text(stmt, 2, date, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, time, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, direction, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, FPM, -1, SQLITE_STATIC); 
        // Execute the request
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            fprintf(stderr, "The request failed: %s\n", sqlite3_errmsg(db_attendance));
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
        sqlite3_close(db_attendance);
        // Release the mutex after performing operations
        pthread_mutex_unlock(&databaseMutex);
    }
}

void DB_find()
{
    // Obtain the mutex before accessing the database
    if (pthread_mutex_lock(&databaseMutex) == 0)
    {
        char *query = "SELECT ID, Date, Time, Direction, FPM "
                      "FROM attendance "
                      "WHERE Saved = 'X';";
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db_attendance, query, -1, &stmt, NULL) != SQLITE_OK)
        {
            fprintf(stderr, "Failed to prepare request: %s\n", sqlite3_errmsg(db_attendance));
            return;
        }
        // Processing query results
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            char https_request[1024];
            int id = sqlite3_column_int(stmt, 0);
            const char *date = (const char *)sqlite3_column_text(stmt, 1);
            const char *time = (const char *)sqlite3_column_text(stmt, 2);
            const char *direction = (const char *)sqlite3_column_text(stmt, 3);
            const char *FPM = (const char *)sqlite3_column_text(stmt, 4);

            // HTTP request
            sprintf(https_request, "id=%d&date=%s&time=%s&direction=%s&FPM=%s", id, date, time, direction, FPM);
            if (send_request(https_request))
            {
                // HTTP request sent successfully
                printf("Data sent to the server: %d, %s, %s, %s, %s\n",id , date, time, direction, FPM);
                DB_update(id);
            }
            else
                printf("Error sending HTTP request\r\n");
        }
        // Finish the request
        sqlite3_finalize(stmt);
        pthread_mutex_unlock(&databaseMutex);
    }
}
// function to change Saved value from 'X' to 'V'
void DB_update(int id)
{
    char updateQuery[256];
    snprintf(updateQuery, sizeof(updateQuery), "UPDATE attendance SET Saved = 'V' WHERE ID = %d;", id);
    char *err_msg = 0;
    int rc = sqlite3_exec(db_attendance, updateQuery, 0, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to update data in the database: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    else
        printf("Data in the database updated: Set 'Saved' to 'V' for ID=%d\n", id);
}
