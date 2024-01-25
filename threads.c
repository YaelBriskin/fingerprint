#include "main.h"

extern int socket;
int uart_fd;

void getCurrentTimeAndDate(char*  dateString , char* timeString) 
{
    // Получаем текущее системное время
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(timeString, 10, "%H:%M", timeinfo);
    strftime(dateString, 12, "%d/%m/%Y", timeinfo);
}

void* fingerPrintThread(void* arg) 
{
    char date[20];
    char time[20];
    // Initialize GPIO for the button and LED
    GPIO_init(GPIO_BUTTON_IN, "in");
    GPIO_init(GPIO_BUTTON_OUT, "in");
    GPIO_init(GPIO_LED, "out");
    //Initialize UART
    uart_fd=UART_Init();

    int button_fd_in=GPIO_open(GPIO_BUTTON_IN);
    int button_fd_out=GPIO_open(GPIO_BUTTON_OUT);
    while (1) 
    {
        if (readGPIO(button_fd_in)) //press button IN
        {
            int id= findFinger("hello");
            if(id)
            {
                getCurrentTimeAndDate(date, time);
                DB_write(id,date,time,"in");
            }
            // Perform actions when button IN is pressed
            printf("Button IN pressed! Turning on LED.\n");
            turnOnLED();
        }
        if (readGPIO(button_fd_out)) //press button OUT
        {
            int id= findFinger("goodbye");
            if(id)
            {
                getCurrentTimeAndDate(date, time);
                DB_write(id,date,time,"out");
            }
            // Perform actions when button OUT is pressed
            printf("Button OUT pressed! Turning on LED.\n");
            turnOnLED();
        }
        usleep(100000); // Pause for 100 milliseconds before the next check
    }
    return NULL;
}
//every two minutes checks whether there are changes in the code and if there are any,sends them to the server
void* databaseThread(void* arg)
{
    while (1) 
    {
        DB_find(socket);
        sleep(120); 
    }
    return NULL;
}