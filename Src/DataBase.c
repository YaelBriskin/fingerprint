#include "../Inc/DataBase.h"

sqlite3 *db_attendance;
pthread_mutex_t databaseMutex;

int getNextAvailableID() 
{
    int id = 0;
    char *query = "SELECT seq FROM sqlite_sequence WHERE name = 'employees'";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db_attendance, query, -1, &stmt, NULL) != SQLITE_OK) 
    {
        syslog_log(LOG_ERR, __func__, "format", "Failed to execute query: %s", sqlite3_errmsg(db_attendance));
        //printf("Failed to execute query: %s\n", sqlite3_errmsg(db_attendance));
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
    char *err_msg = NULL;
    int result;

    // Open a connection to the "attendance" database
    result = sqlite3_open(db_name, &db_attendance);
    if (result != SQLITE_OK)
    {
        syslog_log(LOG_ERR, __func__, "format", "Failed to open attendance database: %s (SQLite error code: %d)", sqlite3_errmsg(db_attendance), result);
        exit(EXIT_FAILURE);
    }

    // Open a connection to the database (if there is no database, it will be created automatically)
    const char *create_attendance_table_query = "CREATE TABLE IF NOT EXISTS attendance ("
                                                "ID INTEGER,"
                                                "Timestamp INTEGER NOT NULL,"
                                                "Direction TEXT NOT NULL,"
                                                "Saved TEXT DEFAULT 'X' NOT NULL,"
                                                "FPM INTEGER NOT NULL,"
                                                "FOREIGN KEY(ID) REFERENCES employees(ID));";

    result = sqlite3_exec(db_attendance, create_attendance_table_query, 0, 0, &err_msg);
    if (result != SQLITE_OK)
    {
        syslog_log(LOG_ERR, __func__, "format", "Failed to create table: %s", err_msg);
        //fprintf(stderr, "Failed to create table: %s\n", err_msg);
        sqlite3_free(err_msg);
        exit(EXIT_FAILURE);
    }
    const char *create_employees_table_query = "CREATE TABLE IF NOT EXISTS employees ("
                                               "ID INTEGER PRIMARY KEY AUTOINCREMENT);";

    result = sqlite3_exec(db_attendance, create_employees_table_query, 0, 0, &err_msg);
    if (result != SQLITE_OK) 
    {
        syslog(LOG_ERR, "Failed to create employees table: %s", err_msg);
        sqlite3_free(err_msg);
        exit(EXIT_FAILURE);
    }
    // Initialize the mutex
    if (pthread_mutex_init(&databaseMutex, NULL) != 0)
    {
        syslog_log(LOG_ERR, __func__, "stderr", "Failed to initialize mutex", NULL);
        exit(EXIT_FAILURE);
    }
}
void DB_newEmployee()
{
    printf("DB_newEmployee()\r\n");
    if (pthread_mutex_lock(&databaseMutex) == 0)
    {
        sqlite3_stmt *stmt;
        const char *sql = "INSERT INTO employees DEFAULT VALUES;";
        if (sqlite3_prepare_v2(db_attendance, sql, -1, &stmt, NULL) != SQLITE_OK)
        {
            syslog(LOG_ERR, "Failed to prepare statement: %s", sqlite3_errmsg(db_attendance));
            return;
        }
        if (sqlite3_step(stmt) != SQLITE_DONE)
            syslog(LOG_ERR, "Failed to insert new employee: %s", sqlite3_errmsg(db_attendance));
        // Finish the request
        sqlite3_finalize(stmt);
    }
    // Release the mutex after performing operations
    pthread_mutex_unlock(&databaseMutex);
}
int DB_write(int ID, int Timestamp, const char *direction,const char *FPM)
{
    // Obtain the mutex before accessing the database
    if (pthread_mutex_lock(&databaseMutex) == 0)
    {
        sqlite3_stmt *stmt;
        // SQL query to insert data into the table
        const char *sql = "INSERT INTO attendance (ID, Timestamp, Direction, FPM) VALUES (?, ?, ?, ?);";
        // Prepare the request
        if (sqlite3_prepare_v2(db_attendance, sql, -1, &stmt, NULL) != SQLITE_OK)
        {
            syslog_log(LOG_ERR, __func__, "format", "Failed to prepare request: %s",sqlite3_errmsg(db_attendance));
            //fprintf(stderr, "Failed to prepare request: %s\n", sqlite3_errmsg(db_attendance));
            pthread_mutex_unlock(&databaseMutex);
            return 0;         
            }
        // Binding values to request parameters
        sqlite3_bind_int(stmt, 1, ID);
        sqlite3_bind_int(stmt, 2, Timestamp);
        sqlite3_bind_text(stmt, 3, direction, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, FPM, -1, SQLITE_STATIC); 
        // Execute the request
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            syslog_log(LOG_ERR, __func__, "format", "The request failed: %s",sqlite3_errmsg(db_attendance));
            //fprintf(stderr, "The request failed: %s\n", sqlite3_errmsg(db_attendance));
            return 0;
        }
        // Finish the request
        sqlite3_finalize(stmt);
        // Release the mutex after performing operations
        pthread_mutex_unlock(&databaseMutex);
        return 1;
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
        char *query = "SELECT ID, Timestamp, Direction, FPM "
                      "FROM attendance "
                      "WHERE Saved = 'X';";
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db_attendance, query, -1, &stmt, NULL) != SQLITE_OK)
        {
            syslog_log(LOG_ERR, __func__, "format", "Failed to prepare request: %s",sqlite3_errmsg(db_attendance));
            //fprintf(stderr, "Failed to prepare request: %s\n", sqlite3_errmsg(db_attendance));
            pthread_mutex_unlock(&databaseMutex);
            return;
        }
        // Processing query results
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            int timestamp = sqlite3_column_int(stmt, 1);
            const char *direction = (const char *)sqlite3_column_text(stmt, 2);
            const char *FPM =(const char *)sqlite3_column_text(stmt, 3);

            // HTTP request
            if(send_json_data(id,direction,timestamp,FPM))
                DB_update(id);
            else
               syslog_log(LOG_ERR, __func__, "stderr", "Error sending HTTP request", NULL);
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
        syslog_log(LOG_ERR, __func__, "format", "Failed to update data in the database: %s", err_msg);
        //fprintf(stderr, "Failed to update data in the database: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}
void DB_delete(int ID)
{
    if (pthread_mutex_lock(&databaseMutex) == 0) 
    {
        // char deleteQuery[256];
        // Create SQL query for deletion
       // snprintf(deleteQuery, sizeof(deleteQuery), "DELETE FROM employees WHERE ID = %d;", ID);
        char *sql_query = NULL;
        // Create SQL query for deletion
        asprintf(&sql_query, "DELETE FROM employees WHERE ID = %d;", ID);
        if (!sql_query) {
            syslog_log(LOG_ERR, __func__, "format", "Memory allocation error");
            return;
        }

        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db_attendance, sql_query, -1, &stmt, NULL) != SQLITE_OK) {
            syslog_log(LOG_ERR, __func__, "format", "Failed to prepare request: %s", sqlite3_errmsg(db_attendance));
            sqlite3_free(sql_query);  // Free allocated memory
            return;                 // Return error code
        }

        // Execute the prepared statement
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            syslog_log(LOG_ERR, __func__, "format", "Failed to delete record: %s", sqlite3_errmsg(db_attendance));
            sqlite3_finalize(stmt);
            sqlite3_free(sql_query);
            return;
        }

        sqlite3_finalize(stmt);
        sqlite3_free(sql_query);
        pthread_mutex_unlock(&databaseMutex);
    }
}
