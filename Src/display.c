#include "../Inc/display.h"

pthread_mutex_t displayMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t messageMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t displayCondition = PTHREAD_COND_INITIALIZER;

bool showMessage = false;
bool delayDisplay =true;

struct LCD_data
{
    char currentMessage[80];
    int row;
    int col;
} LCD_data= { .currentMessage = {'\0'}, .row = -1, .col = -1 };

void showMessageOnDisplay(const char *message, ...)
{
    pthread_mutex_lock(&messageMutex);
    va_list args;
    va_start(args, message); 
    strncpy(LCD_data.currentMessage, message, sizeof(LCD_data.currentMessage));

    if (va_arg(args, int) != -1) 
    { 

        LCD_data.row = va_arg(args, int);
        LCD_data.col = va_arg(args, int);   
    } 
    showMessage = true;
    pthread_mutex_unlock(&messageMutex);
}

void *displayThread(void *arg) 
{
    struct timespec timeout;
    while (1) 
    {
        if (showMessage) 
        {
            lcd20x4_i2c_clear();
            //pthread_mutex_lock(&displayMutex);
            if(LCD_data.row==-1 || LCD_data.col==-1 )
                lcd20x4_i2c_printf(LCD_data.currentMessage);
            else
                lcd20x4_i2c_puts(LCD_data.row, LCD_data.col, LCD_data.currentMessage);
            //pthread_mutex_unlock(&displayMutex);
            if (!delayDisplay)
            {
                showMessage = false;
                LCD_data.currentMessage[0] = '\0'; // Clear the message buffer

            }
        } 
        else 
        {
            // Вычисляем время для ожидания (1 секунда)
            clock_gettime(CLOCK_REALTIME, &timeout);
            timeout.tv_sec += 1;
            // Ожидаем до следующей секунды или пока не появится новое сообщение
            pthread_cond_timedwait(&displayCondition, &displayMutex, &timeout);
            time_t rawtime;
            struct tm *timeinfo;
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            char timeString[20] = {'\0'};
            strftime(timeString, 20, "%X %x", timeinfo);
            lcd20x4_i2c_puts(0, 0, timeString);
        }
    }
    return NULL;
}