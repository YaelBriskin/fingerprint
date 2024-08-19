#include "../Inc/I2C.h"

int i2c_fd;

/**
 * @brief Initializes the I2C interface.
 * @return int 1 on success, 0 on failure.
 */
Status_t I2C_Init()
{
    // Open the I2C bus
    i2c_fd = open(I2C_BUS, O_RDWR);
    if (i2c_fd == ERROR)
    {
        // Log the error if the I2C bus could not be opened
        syslog_log(LOG_ERR, __func__, "strerror", "Error opening I2C", strerror(errno));
        return FAILED;
    }
    // Set the I2C address for all subsequent I2C device transfers
    if (ioctl(i2c_fd, I2C_SLAVE_FORCE, I2C_ADDRESS) < 0)
    {
        // Log the error if the I2C address could not be set
        syslog_log(LOG_ERR, __func__, "strerror", "Error setting I2C address", strerror(errno));
        close(i2c_fd);
        return FAILED;
    }
    return SUCCESS;
}

/**
 * @brief Writes data to the I2C bus.
 * @param buffer The data to write.
 * @param size The size of the data to write.
 */
void I2C_write(uint8_t *buffer, int size)
{
    int retries_I2C_transmit = 0;
    do
    {
        // Attempt to write the data to the I2C bus
        if (write(i2c_fd, buffer, size) == size)
            break;
        syslog_log(LOG_ERR, __func__, "strerr", "Failed to write to I2C bus", NULL);
        retries_I2C_transmit++;
    } while (retries_I2C_transmit < g_max_retries);

    if (retries_I2C_transmit == g_max_retries)
        syslog_log(LOG_ERR, __func__, "strerr", "Error: Maximum retries reached", NULL);
}

/**
 * @brief Closes the I2C interface.
 */
void I2C_close()
{
    close(i2c_fd);
}