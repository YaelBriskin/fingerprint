#include "main.h"

void* buttonThread(void* arg) 
{
    // Initialize GPIO for the button and LED
    GPIO_init(GPIO_BUTTON_1, "in");
    GPIO_init(GPIO_BUTTON_2, "in");
    GPIO_init(GPIO_LED, "out");

    int button_fd1=GPIO_open(GPIO_BUTTON_1);
    int button_fd1=GPIO_open(GPIO_BUTTON_2);
    while (1) 
    {
        int stateButton1 = readGPIO(button_fd1);
        int stateButton2 = readGPIO(button_fd1);

        if (stateButton1 == 1) 
        {
            // Perform actions when button 1 is pressed
            printf("Button 1 pressed! Turning on LED.\n");
            turnOnLED();
        }
        if (stateButton2 == 1) 
        {
            // Perform actions when button 2 is pressed
            printf("Button 2 pressed! Turning on LED.\n");
            turnOnLED();
        }
        usleep(100000); // Pause for 100 milliseconds before the next check
    }
    return NULL;
}

void* uartThread(void* arg) 
{
    int uart_fd=UART_Init();
    Uart_Config(uart_fd);

    while (1) 
    {
        char buffer[10]; 

        // Ваш код обработки прочитанных данных здесь

        usleep(100000); // Пауза на 100 миллисекунд перед следующей проверкой
    }

    return NULL;
}