#include "../Inc/I2C.h"

int i2c_fd;
int I2C_Init()
{
    i2c_fd = open(I2C_BUS, O_RDWR);
    if (i2c_fd == -1) 
    {
        syslog_log(LOG_ERR, __func__, "strerror", "Error opening I2C", strerror(errno));
        //perror("Error opening I2C");
        return 0;
    }
    if (ioctl(i2c_fd, I2C_SLAVE_FORCE, I2C_ADDRESS) < 0)
    {
        syslog_log(LOG_ERR, __func__, "strerror", "Error setting I2C address", strerror(errno));
        //perror("Error setting I2C address");
        close(i2c_fd);
        return 0;
    }
    return 1;
}
void I2C_write(uint8_t *buffer, int size)
{
    int retries_I2C_transmit = 0;
    do
    {
        if (write(i2c_fd, buffer, size) == size)
            break;
        syslog_log(LOG_ERR, __func__, "strerr", "Failed to write to I2C bus", NULL);
        //fprintf(stderr, "Failed to write to I2C bus\n");
        retries_I2C_transmit++;
    } while (retries_I2C_transmit < MAX_RETRIES);

    if (retries_I2C_transmit == MAX_RETRIES)
        syslog_log(LOG_ERR, __func__, "strerr", "Error: Maximum retries reached", NULL);
        //fprintf(stderr, "Error: Maximum retries reached\n");
}

void I2C_close()
{
    close(i2c_fd);
}