#include "../Inc/threads.h"

pthread_mutex_t displayMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t displayCond = PTHREAD_COND_INITIALIZER;
int displayLocked = 0;

int getCurrentUTCTimestamp() 
{
    // Получаем текущее время
    time_t current_time = time(NULL);
    // Возвращаем текущее время в формате UTC
    return (int)current_time;
}

void *fingerPrintThread(void *arg)
{
    int timestamp;
    while (1)
    {
        int button_fd_in = GPIO_open(GPIO_BUTTON_IN);
        int button_fd_out = GPIO_open(GPIO_BUTTON_OUT);
        int button_fd_new = GPIO_open(GPIO_BUTTON_NEW);
        if (button_fd_in == -1 || button_fd_out == -1 || button_fd_new == -1)
        {
            syslog_log(LOG_ERR, __func__, "strerror", "Error opening GPIO value file", strerror(errno));
            // printf("Error opening GPIO value file: %s\n", strerror(errno));
            break;
        }
        // press button
        if (!GPIO_read(button_fd_in)) // press button IN
        {
            pthread_mutex_lock(&displayMutex);
            displayLocked = 1;
            int id = findFinger("Hello");
            timestamp = getCurrentUTCTimestamp();
            if (id)
                DB_write(id, timestamp, "in", "true");
            else if(id == -1)
            {
		        lcd20x4_i2c_puts(1, 0, "No matching in the library");
		        sleep(2);
            }
            else
            {
                id = enter_ID_keypad();
                printf("id=%d\r\n",id);
                if (id > 0)
                {
                    if(DB_write(id, timestamp, "in", "false"))
                    {
                        char mydata[23] = {0};
                        sprintf(mydata, "Hello  ID #%d", id);
                        lcd20x4_i2c_puts(1,0,mydata);
                        sleep(2);
                    }
                    else
                    lcd20x4_i2c_puts(1,0,"Failed to write to database");
                    sleep(2);
                }
            }
            displayLocked = 0; 
            pthread_cond_signal(&displayCond); // Отправляем сигнал о завершении работы с дисплеем
            lcd20x4_i2c_clear();
            pthread_mutex_unlock(&displayMutex);
            // Perform actions when button IN is pressed
        }
        if (!GPIO_read(button_fd_out)) // press button OUT
        {
            pthread_mutex_lock(&displayMutex);
            displayLocked = 1;            
            int id = findFinger("Goodbye");
            timestamp = getCurrentUTCTimestamp();
            if (id)
                DB_write(id, timestamp, "out", "true");
            else if(id == -1)
            {
		        lcd20x4_i2c_puts(1, 0, "No matching in the library");
		        sleep(2);
            }
            else
            {
                id = enter_ID_keypad();
                printf("id=%d\r\n",id);
                if (id > 0)
                {
                    if(DB_write(id, timestamp, "out", "false"))
                    {
                        char mydata[23] = {0};
                        sprintf(mydata, "Goodbye  ID #%d", id);
                        lcd20x4_i2c_puts(1,0,mydata);
                        sleep(2);
                    }
                    else
                    lcd20x4_i2c_puts(1,0,"Failed to write to database");
                    sleep(2);
                }
            }
            displayLocked = 0; 
            pthread_cond_signal(&displayCond); // Отправляем сигнал о завершении работы с дисплеем
            lcd20x4_i2c_clear();
            pthread_mutex_unlock(&displayMutex);
            // Perform actions when button OUT is pressed
        }
        if (!GPIO_read(button_fd_new)) // press button NEW (new employee)
        {
            int id = getNextAvailableID();
            printf("id =%d",id);
            pthread_mutex_lock(&displayMutex);
            displayLocked = 1;
            int ack=enrolling(id);
            if (ack)
            {
                DB_newEmployee();
                char messageString[50];
                sprintf(messageString, "Employee %d added successfully.", id);
                printf("%s\r\n",messageString);
                lcd20x4_i2c_puts(0,0,messageString);
                send_json_new_employee (id);
                sleep(3);
            }
            else
            {
                // If enrolling fails, remove the entry from the database
                syslog_log(LOG_ERR, __func__, "format", "Enrolling failed.");
                lcd20x4_i2c_puts(1,0,"Enrolling failed.");
            }

            displayLocked = 0; 
            pthread_cond_signal(&displayCond); // Отправляем сигнал о завершении работы с дисплеем
            lcd20x4_i2c_clear();
            pthread_mutex_unlock(&displayMutex);
        }
        usleep(10000);
        GPIO_close(GPIO_BUTTON_IN);
        GPIO_close(GPIO_BUTTON_OUT);
        GPIO_close(GPIO_BUTTON_NEW);
    }
}
// every two minutes checks whether there are changes in the code and if there are any,sends them to the server
void *databaseThread(void *arg)
{
    while (1)
    {
        DB_find();
        sleep(120);
    }
    return NULL;
}

void *clockThread(void *arg)
{
    struct timespec timeout;
    while (1)
    {
        time_t rawtime;
        struct tm *timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        char timeString[20] = {'\0'};
        strftime(timeString, 20, "%X %d/%m/%y", timeinfo);
        pthread_mutex_lock(&displayMutex);
        while (displayLocked) 
        {            
            //pthread_cond_wait(&displayCond, &displayMutex);
            clock_gettime(CLOCK_REALTIME, &timeout);
            timeout.tv_sec += 1;
            // Ждем сигнала или истечения таймера
            int result = pthread_cond_timedwait(&displayCond, &displayMutex, &timeout);
            // Если время истекло, выходим из цикла ожидания
            if (result == ETIMEDOUT) 
                break;
        }
        lcd20x4_i2c_puts(0, 0, timeString);
        pthread_mutex_unlock(&displayMutex);
        //sleep(1);
    }
}
