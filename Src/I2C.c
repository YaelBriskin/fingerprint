#include "../Inc/I2C.h"

int i2c_fd;
int I2C_Init()
{
    i2c_fd = open(I2C_BUS, O_RDWR);
    if (i2c_fd == -1) 
    {
        perror("Error opening I2C");
        return 0;
    }
    if (ioctl(i2c_fd, I2C_SLAVE_FORCE, I2C_ADDRESS) < 0)
    {
        perror("Error setting I2C address");
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
        perror("Failed to write to I2C bus");
        retries_I2C_transmit++;
    } while (retries_I2C_transmit < MAX_RETRIES);

    if (retries_I2C_transmit == MAX_RETRIES)
    {
        perror("Error: Maximum retries reached");
        fprintf(stderr, "Error: Maximum retries reached\n");
    }
}

void I2C_close()
{
    close(i2c_fd);
}