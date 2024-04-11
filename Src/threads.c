#include "../Inc/threads.h"


void getCurrentTimeAndDate(char *dateString, char *timeString)
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
        if (GPIO_init(GPIO_BUTTON_IN, "in") == 1 && GPIO_init(GPIO_BUTTON_OUT, "in") == 1 && GPIO_init(GPIO_BUTTON_NEW, "in") == 1)
        {
            printf("GPIO initialization successful!\n");
            // Initialize UART
            if (UART_Init(UART2_DEVICE, UART2_BaudRate) && UART_Init(UART4_DEVICE, UART4_BaudRate))
            {
                printf("UART initialization successful!\n");
                while (1)
                {
                    int button_fd_in = GPIO_open(GPIO_BUTTON_IN);
                    int button_fd_out = GPIO_open(GPIO_BUTTON_OUT);
                    int button_fd_new = GPIO_open(GPIO_BUTTON_NEW);
                    if (button_fd_in == -1 || button_fd_out == -1 || button_fd_new == -1)
                    {
                        printf("Error opening GPIO value file: %s\n", strerror(errno));
                        break;
                    }
                    // press button
                    if (!GPIO_read(button_fd_in)) // press button IN
                    {
                        printf("Button IN pressed!\n");
                        int id = findFinger("Hello");
                        if (id)
                        {
                            getCurrentTimeAndDate(date, time);
                            DB_write(id, date, time, "in", "V");
                        }
                        else
                        {
                            int id=enter_ID_keypad();
                            DB_write(id, date, time, "in", "X");

                        }
                        // Perform actions when button IN is pressed
                    }
                    if (!GPIO_read(button_fd_out)) // press button OUT
                    {
                        printf("Button OUT pressed!\n");
                        int id = findFinger("Goodbye");
                        if (id)
                        {
                            getCurrentTimeAndDate(date, time);
                            DB_write(id, date, time, "out","V");
                        }
                        else
                        {
                            int id=enter_ID_keypad();
                            DB_write(id, date, time, "out", "X");

                        }
                        // Perform actions when button OUT is pressed
                    }
                    if (!GPIO_read(button_fd_new)) // press button NEW (new employee)
                    {
                        printf("Button NEW pressed!\n");
                        int id =getNextAvailableID();
                        printf("id =%d\n", id);
                        if (enrolling(id) == 0)
                        {
                            // If enrolling fails, remove the entry from the database
                            printf("Enrolling failed.Keypad input...\n");
                            enter_ID_keypad();
                        }
                    }
                    usleep(200000);
                    GPIO_close(GPIO_BUTTON_IN);
                    GPIO_close(GPIO_BUTTON_OUT);
                    GPIO_close(GPIO_BUTTON_NEW);
                }
            }
            else
                printf("UART initialization failed: %s\n", strerror(errno));
        }
        else
            printf("GPIO initialization failed: %s\n", strerror(errno));
        return NULL;
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


