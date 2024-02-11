#include "../Inc/threads.h"

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

void *fingerPrintThread(void *arg)
{
    while (1)
    {
        char date[20] = {0};
        char time[20] = {0};
        // Initialize GPIO for the button and LED
        if (GPIO_init(GPIO_BUTTON_IN, "in") != 1 || GPIO_init(GPIO_BUTTON_OUT, "in") != 1 || GPIO_init(GPIO_LED, "out") != 1)
        {
            printf("GPIO initialization failed!\n");
            return NULL;
        }
        printf("GPIO initialization successful!\n");

        // Initialize UART
        UART_Init();

        int button_fd_in = GPIO_open(GPIO_BUTTON_IN);
        int button_fd_out = GPIO_open(GPIO_BUTTON_OUT);
        while (1)
        {
            if (GPIO_read(button_fd_in)) // press button IN
            {
                int id = findFinger("Hello");
                if (id)
                {
                    getCurrentTimeAndDate(date, time);
                    DB_write(id, date, time, "in");
                }
                // Perform actions when button IN is pressed
                printf("Button IN pressed! Turning on LED.\n");
                turnOnLED();
            }
            if (GPIO_read(button_fd_out)) // press button OUT
            {
                int id = findFinger("Goodbye");
                if (id)
                {
                    getCurrentTimeAndDate(date, time);
                    DB_write(id, date, time, "out");
                }
                // Perform actions when button OUT is pressed
                printf("Button OUT pressed! Turning on LED.\n");
                turnOnLED();
            }
            usleep(100000); // Pause for 100 milliseconds before the next check
        }
        return NULL;
    }
}
//every two minutes checks whether there are changes in the code and if there are any,sends them to the server
void* databaseThread(void* arg)
{
    while (1) 
    {
        DB_find();
        sleep(120); 
    }
    return NULL;
}

void* socketThread(void* arg) 
{
    char buffer[SIZE_Eth];
    while (1) 
    {
        // Receive data
        socket_read(buffer,SIZE_Eth); 
        usleep(500000); 
    }
}
