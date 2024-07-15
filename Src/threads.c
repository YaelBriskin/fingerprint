#include "../Inc/threads.h"
//-------------display
pthread_mutex_t displayMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t displayCond = PTHREAD_COND_INITIALIZER;
//-------------database
pthread_cond_t databaseCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t databaseMutex = PTHREAD_MUTEX_INITIALIZER;

int displayLocked = UNLOCK;

/**
 * @brief This function returns the current time in UTC format as an integer timestamp.
 *
 * @return The current UTC time as an integer timestamp.
 */
int getCurrent_UTC_Timestamp() 
{
    // Get the current time
    time_t current_time = time(NULL);
    return (int)current_time;
}
/**
 * @brief This function activates a buzzer for a short duration.
 *
 */
void buzzer()
{
    int fd_buzzer = GPIO_open(GPIO_BUZZER, O_WRONLY);
    GPIO_write(fd_buzzer,BUZZER_ON);
    usleep(SLEEP_BUZZER);
    GPIO_write(fd_buzzer,BUZZER_OFF);
    GPIO_close(GPIO_BUZZER);
}
/**
 * @brief This function runs in a separate thread to periodically check for unsent data in the database and send it to the server.
 *
 * The function operates in an infinite loop, checking every two minutes if there is data in the database that has not been sent.
 *  - Sleeps for 600 seconds before repeating the process.
 * @param arg Unused parameter.
 * @return Always returns NULL.
 */
void *databaseThread(void *arg)
{
    int count = 0;
    struct timespec timeout;

    while (1)
    {
        int fd_led = GPIO_open(GPIO_LED_RED,O_WRONLY);
        //checks whether there is data in the database that has not yet been sent and 
        //if there is any, it sends it to the server
        if (DB_find()!= 1)
        {
            //if it fails to send data 10 times in a row, the LED will light up
            if (++count == g_max_retries)
            {
                //led on
                GPIO_write(fd_led,LED_ON);
            }

        }
        else
        {
            count = 0;
            //led off
            GPIO_write(fd_led,LED_OFF);
        }
        GPIO_close(GPIO_LED_RED);
        //sleep(DATABASE_SLEEP_DURATION);
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += g_db_sleep;  

        pthread_mutex_lock(&databaseMutex);
        pthread_cond_timedwait(&databaseCond, &databaseMutex, &timeout);
        pthread_mutex_unlock(&databaseMutex);
    }
    return NULL;
}
/**
 * @brief This function runs in a separate thread to update the display with the current time and manage database records.
 * @param arg Unused parameter.
 * @return Always returns NULL.
 */
void *clockThread(void *arg)
{
    struct timespec timeout;
    int lastDay = -1;

    while (1)
    {
        time_t rawtime;
        struct tm *timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);

        // If the day has changed since the last check, it deletes old records from the database.
        if (lastDay != -1 && lastDay != timeinfo->tm_mday)
        {
            DB_delete_old_records(rawtime);
        }

        //Updates the last checked day
        lastDay = timeinfo->tm_mday;

        char timeString[TIME_STR_LEN] = {'\0'};
        strftime(timeString, TIME_STR_LEN, "%X %d/%m/%y", timeinfo);
        pthread_mutex_lock(&displayMutex);
        while (displayLocked) 
        {            
            //pthread_cond_wait(&displayCond, &displayMutex);
            clock_gettime(CLOCK_REALTIME, &timeout);
            timeout.tv_sec += ONE_MINUTE;
            // Wait for a signal or timer to expire
            int result = pthread_cond_timedwait(&displayCond, &displayMutex, &timeout);
            // If time has expired, exit the wait loop
            if (result == ETIMEDOUT) 
            {
                break;
            }
        }
        lcd20x4_i2c_puts(0, 0, timeString);//Updates the display with the current time.
        pthread_mutex_unlock(&displayMutex);
    }
}
/**
 * @brief Thread function for handling server socket operations.
 *
 * @param arg A pointer to thread arguments (not used in this implementation).
 * @return void* NULL pointer.
 */
void *socket_serverThread (void *arg)
{
    int server_socket = create_server_socket();
    if (server_socket == -1) 
    {
        syslog_log(LOG_ERR, __func__, "strerror", "Error creating server socket: ", strerror(errno));
        return NULL; // Handle error in creating socket
    }
    while (1)  
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);

        if (client_socket < 0) 
        {
            syslog_log(LOG_ERR, __func__, "strerror", "Error accepting connection: ", strerror(errno));
        }
        else 
        {
            // Spawn a new thread to handle the client
            pthread_t client_thread;
            if (pthread_create(&client_thread, NULL, handle_clientThread, (void *)client_socket) != THREAD_OK) 
            {
                syslog_log(LOG_ERR, __func__, "strerror", " Error creating client thread: ", strerror(errno));
                close(client_socket);
            } 
            else 
            {
                // Detach the client thread 
                pthread_detach(client_thread);
            }
        }
    }
}
/**
 * @brief Thread function for handling client operations.
 *
 * @param arg A pointer to the client socket file descriptor.
 * @return void* NULL pointer.
 */
void *handle_clientThread(void *arg) 
{
    int client_socket = (int)arg;
    int client_id;
    while (1) 
    {
        int received_bytes = read_data_from_client(client_socket, &client_id);
        if (received_bytes == FAILED) 
        {
            syslog_log(LOG_ERR, __func__, "strerror", "  Error reading from client:  ", strerror(errno));
            break;
        } 
        else 
        {
            //remove the received fingerprint from the database
            deleteModel(client_id);
            DB_delete(client_id);
        }
    }
    close(client_socket);
}