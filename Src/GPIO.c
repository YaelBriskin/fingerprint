#include "../Inc/GPIO.h"

int GPIO_init(int pinNumber, const char* direction) 
{
    char gpioPath[50];
    char direction_path[50];

    snprintf(gpioPath, sizeof(gpioPath), "/sys/class/gpio/gpio%d", pinNumber);

    // Check if the GPIO folder already exists
    if (access(gpioPath, F_OK) != 0) 
    {
        // If the folder does not exist, create it
        int export_fd = open("/sys/class/gpio/export", O_WRONLY);
        if (export_fd == -1) 
        {
            syslog_log(LOG_ERR, __func__, "strerror", "Error opening GPIO export", strerror(errno));
            //perror("Error opening GPIO export");
            return 0;
        }

        // export GPIO
        char pinBuffer[4];
        snprintf(pinBuffer, sizeof(pinBuffer), "%d", pinNumber);
        write(export_fd, pinBuffer, strlen(pinBuffer));
        close(export_fd);
    }

    snprintf(direction_path, sizeof(direction_path), "/sys/class/gpio/gpio%d/direction", pinNumber);
    // Open the file direction
    int gpio_fd = open(direction_path, O_WRONLY);
    if (gpio_fd == -1) 
    {
        syslog_log(LOG_ERR, __func__, "strerror", "Error opening GPIO direction", strerror(errno));
        //perror("Error opening GPIO direction");
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
        syslog_log(LOG_ERR, __func__, "strerror", "Error reading GPIO value", strerror(errno));
        //perror("Error reading GPIO value");
        close(gpio_fd);
        exit(EXIT_FAILURE);
    }
    return (value == '1') ? 1 : 0;
}

int GPIO_open(int pinNumber)
{
    char gpioPath[50];
    snprintf(gpioPath, sizeof(gpioPath), "/sys/class/gpio/gpio%d/value", pinNumber);
    int fd = open(gpioPath, O_RDONLY);
    if (fd == -1) 
    {
        syslog_log(LOG_ERR, __func__, "strerror", "Error opening GPIO value file", strerror(errno));
        //perror("Error opening GPIO value file");
        exit(EXIT_FAILURE);
    }
    return fd;
}

void GPIO_close(int gpio_fd)
{
    close(gpio_fd);
}