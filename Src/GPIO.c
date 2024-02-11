#include "../Inc/GPIO.h"

int GPIO_init(int pinNumber, const char* direction) 
{
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d", pinNumber);

    // Check if the GPIO folder already exists
    if (access(buffer, F_OK) != 0) 
    {
        // If it doesn't exist, create the GPIO folder
        snprintf(buffer, sizeof(buffer), "echo %d > /sys/class/gpio/export", pinNumber);
        system(buffer);
    }

    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/direction", pinNumber);
    int gpio_fd = open(buffer, O_WRONLY);
    if (gpio_fd == -1) 
    {
        perror("Error opening GPIO direction");
        return 0;
    }

    // Set the GPIO direction
    write(gpio_fd, direction, strlen(direction));
    close(gpio_fd);
    return 1;
}

int GPIO_read(int gpio_fd)
{
    char value;
    if (read(gpio_fd, &value, sizeof(value)) == -1)
    {
        perror("Error reading GPIO value");
        close(gpio_fd);
        exit(EXIT_FAILURE);
    }
    return (value == '1') ? 1 : 0;
}
void turnOnLED() 
{
    int ledState = 1;
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", GPIO_LED);
    int led_fd = open(buffer, O_WRONLY);
    if (led_fd == -1) {
        perror("Error opening LED value file");
        exit(EXIT_FAILURE);
    }

    write(led_fd, &ledState, sizeof(ledState));
    close(led_fd);
}

int GPIO_open(int pinNumber)
{
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", pinNumber);
    int fd = open(buffer, O_RDONLY);
    if (fd == -1) 
    {
        perror("Error opening GPIO value file");
        exit(EXIT_FAILURE);
    }
    return fd;
}

void GPIO_close(int gpio_fd)
{
    close(gpio_fd);
}