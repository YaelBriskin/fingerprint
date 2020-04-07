#include "main.h"

extern int socket;
extern Database DB;
//extern int fingerFlag;

void* buttonThread(void* arg) 
{
    // Initialize GPIO for the button and LED
    GPIO_init(GPIO_BUTTON_IN, "in");
    GPIO_init(GPIO_BUTTON_OUT, "in");
    GPIO_init(GPIO_LED, "out");

    int button_fd_in=GPIO_open(GPIO_BUTTON_IN);
    int button_fd_out=GPIO_open(GPIO_BUTTON_OUT);
    while (1) 
    {
        int stateButton_in = readGPIO(button_fd_in);
        int stateButton_out = readGPIO(button_fd_out);

        if (stateButton_in == 1) 
        {
            // Perform actions when button IN is pressed
            printf("Button IN pressed! Turning on LED.\n");
            turnOnLED();
        }
        if (stateButton_out == 1) 
        {
            // Perform actions when button OUT is pressed
            printf("Button OUT pressed! Turning on LED.\n");
            turnOnLED();
        }
        usleep(100000); // Pause for 100 milliseconds before the next check
    }
    return NULL;
}

void* uartThread(void* arg) 
{
    int uart_fd=UART_Init();
    while (1) 
    {
        UART_write(uart_fd, "f", 1);
        char buffer[10]; 

        // Ваш код обработки прочитанных данных здесь

        usleep(100000); 
    }

    return NULL;
}

void* databaseThread(void* arg) 
{
    while (1) 
    {
        DB_find(DB,socket);
        sleep(120); 
    }
    return NULL;
}