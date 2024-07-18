#include "../Inc/DataBase.h"

sqlite3 *db_attendance;
pthread_mutex_t sqlMutex;

/**
 * @brief Retrieves the next available ID from the database.
 * @return The next available ID.
 */
int getNextAvailableID()
{
    int id = 0;
    char *query = "SELECT seq FROM sqlite_sequence WHERE name = 'employees'";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db_attendance, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        syslog_log(LOG_ERR, __func__, "format", "Failed to execute query: %s", sqlite3_errmsg(db_attendance));
        return id;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW)
        id = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return id + 1;
}
/**
 * @brief Opens the attendance database and initializes the required tables.
 *
 * This function opens a connection to the 'employee_attendance.db' database.
 * If the database does not exist, it will be created automatically. It also
 * creates the 'attendance' and 'employees' tables if they do not already exist.
 */
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
        // fprintf(stderr, "Failed to create table: %s\n", err_msg);
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
    if (pthread_mutex_init(&sqlMutex, NULL) != MUTEX_OK)
    {
        syslog_log(LOG_ERR, __func__, "stderr", "Failed to initialize mutex", NULL);
        exit(EXIT_FAILURE);
    }
}
/**
 * @brief Adds a new employee to the database.
 *
 * This function inserts a new record into the 'employees' table. The ID is
 * automatically generated by the database.
 */
void DB_newEmployee()
{
    if (pthread_mutex_lock(&sqlMutex) == MUTEX_OK)
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
    pthread_mutex_unlock(&sqlMutex);
}
/**
 * @brief Writes an attendance record to the database.
 *
 * This function inserts a new record into the 'attendance' table.
 *
 * @param ID The ID of the employee.
 * @param Timestamp The timestamp of the attendance record.
 * @param direction The direction of the attendance ("in" or "out").
 * @param FPM The fingerprint match status ("true" or "false").
 * @return 1 on success, 0 on failure.
 */
Status_t DB_write(int ID, int Timestamp, const char *direction, const char *FPM)
{
    // Obtain the mutex before accessing the database
    if (pthread_mutex_lock(&sqlMutex) == MUTEX_OK)
    {
        sqlite3_stmt *stmt;
        // SQL query to insert data into the table
        const char *sql = "INSERT INTO attendance (ID, Timestamp, Direction, FPM) VALUES (?, ?, ?, ?);";
        // Prepare the request
        if (sqlite3_prepare_v2(db_attendance, sql, -1, &stmt, NULL) != SQLITE_OK)
        {
            syslog_log(LOG_ERR, __func__, "format", "Failed to prepare request: %s", sqlite3_errmsg(db_attendance));
            pthread_mutex_unlock(&sqlMutex);
            return FAILED;
        }
        // Binding values to request parameters
        sqlite3_bind_int(stmt, 1, ID);
        sqlite3_bind_int(stmt, 2, Timestamp);
        sqlite3_bind_text(stmt, 3, direction, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, FPM, -1, SQLITE_STATIC);
        // Execute the request
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            syslog_log(LOG_ERR, __func__, "format", "The request failed: %s", sqlite3_errmsg(db_attendance));
            return FAILED;
        }
        // Finish the request
        sqlite3_finalize(stmt);
        // Release the mutex after performing operations
        pthread_mutex_unlock(&sqlMutex);
        return SUCCESS;
    }
}
/**
 * @brief Closes the connection to the database.
 *
 * This function closes the connection to the 'employee_attendance.db' database
 * and destroys the mutex.
 */
void DB_close()
{
    // Obtain the mutex before accessing the database
    if (pthread_mutex_lock(&sqlMutex) == MUTEX_OK)
    {
        // Close the connection to the database
        sqlite3_close(db_attendance);
        // Release the mutex after performing operations
        pthread_mutex_unlock(&sqlMutex);
    }
}
/**
 * @brief Finds unsent attendance records in the database and sends them to the server.
 *
 * This function queries the 'attendance' table for records where the 'Saved' column
 * is 'X', indicating they have not been sent to the server. It attempts to send these
 * records to the server and updates the 'Saved' column to 'V' upon success.
 *
 * @return 1 if there were records sent successfully, -1 on failure, 0 if no records were found.
 */
int DB_find()
{
    // Obtain the mutex before accessing the database
    if (pthread_mutex_lock(&sqlMutex) == MUTEX_OK)
    {
        char *query = "SELECT ID, Timestamp, Direction, FPM FROM attendance WHERE Saved = 'X';";
        sqlite3_stmt *stmt;
        int check = 0;
        if (sqlite3_prepare_v2(db_attendance, query, -1, &stmt, NULL) != SQLITE_OK)
        {
            syslog_log(LOG_ERR, __func__, "format", "Failed to prepare request: %s", sqlite3_errmsg(db_attendance));
            pthread_mutex_unlock(&sqlMutex);
            return ERROR;
        }
        // Processing query results
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            int timestamp = sqlite3_column_int(stmt, 1);
            const char *direction = (const char *)sqlite3_column_text(stmt, 2);
            const char *FPM = (const char *)sqlite3_column_text(stmt, 3);

            // HTTP request
            if (send_json_data(id, direction, timestamp, FPM) == SUCCESS)
            {
                DB_update(id);
                check = 1;
            }
            else
                syslog_log(LOG_ERR, __func__, "stderr", "Error sending HTTP request", NULL);
        }
        // Finish the request
        sqlite3_finalize(stmt);
        pthread_mutex_unlock(&sqlMutex);
        return check;
    }
    else
    {
        syslog_log(LOG_ERR, __func__, "Failed to lock mutex", NULL);
        return ERROR;
    }
}
/**
 * @brief Updates the 'Saved' status of a record to 'V'.
 *
 * This function updates the 'Saved' column of a record in the 'attendance' table
 * to 'V' indicating that it has been successfully sent to the server.
 *
 * @param id The ID of the record to update.
 */
void DB_update(int id)
{
    char updateQuery[256];
    snprintf(updateQuery, sizeof(updateQuery), "UPDATE attendance SET Saved = 'V' WHERE ID = %d;", id);
    char *err_msg = 0;
    int rc = sqlite3_exec(db_attendance, updateQuery, 0, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        syslog_log(LOG_ERR, __func__, "format", "Failed to update data in the database: %s", err_msg);
        // fprintf(stderr, "Failed to update data in the database: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}
/**
 * @brief Deletes an employee record from the database.
 *
 * This function deletes a record from the 'employees' table based on the specified ID.
 *
 * @param ID The ID of the employee to delete.
 */
void DB_delete(int ID)
{
    if (pthread_mutex_lock(&sqlMutex) == MUTEX_OK)
    {
        char *sql_query = NULL;
        // Create SQL query for deletion
        int ret = asprintf(&sql_query, "DELETE FROM employees WHERE ID = %d;", ID);
        if (!sql_query || ret == -1)
        {
            syslog_log(LOG_ERR, __func__, "format", "Memory allocation error");
            return;
        }

        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db_attendance, sql_query, -1, &stmt, NULL) != SQLITE_OK)
        {
            syslog_log(LOG_ERR, __func__, "format", "Failed to prepare request: %s", sqlite3_errmsg(db_attendance));
            sqlite3_free(sql_query); // Free allocated memory
            pthread_mutex_unlock(&sqlMutex);
            return; // Return error code
        }

        // Execute the prepared statement
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            syslog_log(LOG_ERR, __func__, "format", "Failed to delete record: %s", sqlite3_errmsg(db_attendance));
            sqlite3_finalize(stmt);
            sqlite3_free(sql_query);
            pthread_mutex_unlock(&sqlMutex);
            return;
        }
        writeToFile(__func__, "ID deleted from DB");
        sqlite3_finalize(stmt);
        sqlite3_free(sql_query);
        pthread_mutex_unlock(&sqlMutex);
    }
}
/**
 * @brief Deletes old attendance records from the database.
 *
 * This function deletes records from the 'attendance' table where the timestamp
 * is older than two months from the specified time.
 *
 * @param lastDay The time threshold for deleting old records.
 */
void DB_delete_old_records(time_t lastDay)
{
    struct tm *timeinfo = localtime(&lastDay);
    timeinfo->tm_mon -= g_month; // Subtract two months
    mktime(timeinfo);

    time_t timestamp_threshold = mktime(timeinfo);

    char *sql_query = NULL;
    asprintf(&sql_query, "DELETE FROM attendance WHERE Timestamp < %ld;", (long)timestamp_threshold);
    if (!sql_query)
    {
        syslog(LOG_ERR, __func__, "format", "Memory allocation error");
        return;
    }
    if (pthread_mutex_lock(&sqlMutex) == MUTEX_OK)
    {
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db_attendance, sql_query, -1, &stmt, NULL) != SQLITE_OK)
        {
            syslog(LOG_ERR, __func__, "format", "Failed to prepare request: %s", sqlite3_errmsg(db_attendance));
            sqlite3_free(sql_query);
            pthread_mutex_unlock(&sqlMutex);
            return;
        }
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            syslog(LOG_ERR, __func__, "format", "Failed to delete records: %s", sqlite3_errmsg(db_attendance));
            sqlite3_finalize(stmt);
            sqlite3_free(sql_query);
            pthread_mutex_unlock(&sqlMutex);
            return;
        }

        sqlite3_finalize(stmt);
        sqlite3_free(sql_query);
        pthread_mutex_unlock(&sqlMutex);
    }
}
int DB_check_id_exists(int id)
{
    if (pthread_mutex_lock(&sqlMutex) == MUTEX_OK)
    {
        const char *query = "SELECT 1 FROM employees WHERE ID = ? LIMIT 1;";
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db_attendance, query, -1, &stmt, NULL) != SQLITE_OK)
        {
            syslog_log(LOG_ERR, __func__, "format", "Failed to prepare request: %s", sqlite3_errmsg(db_attendance));
            pthread_mutex_unlock(&sqlMutex);
            return ERROR;
        }
        sqlite3_bind_int(stmt, 1, id);
        int result = sqlite3_step(stmt);
        if (result == SQLITE_ROW)
        {
            sqlite3_finalize(stmt);
            return SUCCESS; // ID найден
        }
        else
        {
            sqlite3_finalize(stmt);
            return FAILED; // ID не найден
        }
    }
}