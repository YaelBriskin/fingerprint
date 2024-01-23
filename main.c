#include "main.h"


int main() 
{
    // Initialize I2C Display
    lcd16x2_i2c_init();

    // Create a thread for checking the button state
    pthread_t thread;
    if (pthread_create(&thread, NULL, buttonThread, NULL) != 0) 
    {
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }
}