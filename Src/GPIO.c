#include "../Inc/GPIO.h"


/**
 * @brief Initializes a GPIO pin with the specified direction.
 * @param pinNumber The number of the GPIO pin to initialize.
 * @param direction The direction of the GPIO pin ("in" or "out").
 * @return 1 on success, 0 on failure.
 */
Status_t GPIO_init(int pinNumber, const char* direction) 
{
    char gpioPath[50];
    char direction_path[50];

    snprintf(gpioPath, sizeof(gpioPath), "/sys/class/gpio/gpio%d", pinNumber);

    // Check if the GPIO folder already exists
    if (access(gpioPath, F_OK) != 0) 
    {
        // If the folder does not exist, create it
        int export_fd = open("/sys/class/gpio/export", O_WRONLY);
        if (export_fd == ERROR) 
        {
            LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error opening GPIO export", strerror(errno));
            return FAILED;
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
    if (gpio_fd == ERROR) 
    {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error opening GPIO direction", strerror(errno));
        return FAILED;
    }

    // Set the GPIO direction
    write(gpio_fd, direction, strlen(direction));
    close(gpio_fd);
    return SUCCESS;
}
/**
 * @brief Reads the value of a GPIO pin.
 *
 * This function reads the current value of a GPIO pin. The value is read
 * from the file descriptor associated with the GPIO pin.
 *
 * @param gpio_fd The file descriptor of the GPIO pin.
 * @return The value of the GPIO pin (1 or 0).
 */

int GPIO_read(int gpio_fd)
{
    char value;
    if (read(gpio_fd, &value, sizeof(value)) == ERROR)
    {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error reading GPIO value", strerror(errno));
        close(gpio_fd);
        exit(EXIT_FAILURE);
    }
    return (value == '1') ? 1 : 0;
}
/**
 * @brief Writes a value to a GPIO pin.
 *
 * This function writes a value to a GPIO pin. The value is written
 * to the file descriptor associated with the GPIO pin.
 *
 * @param gpio_fd The file descriptor of the GPIO pin.
 * @param value The value to write to the GPIO pin (1 or 0).
 */
void GPIO_write(int gpio_fd, int value) 
{
    char val_str = (value == 1) ? '1' : '0';
    if (write(gpio_fd, &val_str, 1) == ERROR) 
    {
        LOG_MESSAGE(LOG_ERR, __func__,  "strerror" ,"Error writing GPIO value", strerror(errno));
        //exit(EXIT_FAILURE);
    }
}
/**
 * @brief Opens a GPIO pin for reading or writing.
 *
 * This function opens the value file of a GPIO pin with the specified flag.
 *
 * @param pinNumber The number of the GPIO pin to open.
 * @param flag The flag for opening the GPIO pin (O_RDONLY or O_WRONLY).
 * @return The file descriptor of the GPIO pin.
 */
int GPIO_open(int pinNumber, int flag)
{
    char gpioPath[50];
    snprintf(gpioPath, sizeof(gpioPath), "/sys/class/gpio/gpio%d/value", pinNumber);
    int fd = open(gpioPath, flag);
    if (fd == ERROR) 
    {
        LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Error opening GPIO value file", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fd;
}
/**
 * @brief Closes a GPIO pin.
 *
 * This function closes the file descriptor associated with a GPIO pin.
 *
 * @param gpio_fd The file descriptor of the GPIO pin.
 */
void GPIO_close(int gpio_fd)
{
    close(gpio_fd);
}