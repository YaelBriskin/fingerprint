#include "../Inc/keypad.h"

extern int uart4_fd;

// Define the buffer for the entered ID
char code[MAX_LENGTH_ID + 1] = {'_', '_', '_', '\0'};

/**
 * @brief Initializes and displays the initial keypad entry screen.
 */
void beginDisplay()
{
    lcd20x4_i2c_clear();
    // Заполнение массива code символами '_'
    for (int i = 0; i < MAX_LENGTH_ID; i++) {
        code[i] = '_';
    }
    // Добавление нулевого символа в конец строки для обозначения конца
    code[MAX_LENGTH_ID] = '\0';
    lcd20x4_i2c_print(0, 0, "Enter ID: ");
    lcd20x4_i2c_print(1, 9, code);
    lcd20x4_i2c_print(2, 0, "* delete/cancel");
    lcd20x4_i2c_print(3, 0, "# confirm");
    
}
/**
 * @brief Reads and validates an ID entered through the keypad.
 *
 * This function waits for the user to enter an ID using the keypad, validates it, 
 * and returns the ID if valid. It returns -1 if a timeout occurs.
 *
 * @return int The entered ID or -1 on timeout.
 */
int enter_ID_keypad()
{
    beginDisplay();
    struct timespec start_time;
    const int max_execution_time = 60;// Maximum time allowed for input in seconds
    struct timespec current_time;
    int star_count = 0; 
    uint8_t rx_buffer;
    int digit_count = 0;
    int attempts = 0;

    // Record the start time
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    while (1)
    {
        // Calculate elapsed time
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        long elapsed_time = (current_time.tv_sec - start_time.tv_sec) + (current_time.tv_nsec - start_time.tv_nsec) / 1000000000;
        if (elapsed_time >= max_execution_time) 
        {
            lcd20x4_i2c_puts(1, 0,"Timeout: One minute has passed.");
            sleep(SLEEP_DURATION);
            lcd20x4_i2c_clear();
            return ERROR;
        }
        // Read from UART
        if(UART_read(uart4_fd, &rx_buffer, 1) == SUCCESS)
        {
            char character = convert_to_char(rx_buffer);
            // Handle character input
            if (character != '\0')
            {
                if (character != '#' && character != '*' && digit_count < MAX_LENGTH_ID)
                {

                    code[digit_count] = character;
                    digit_count++;
                    // Update LCD display
                    lcd20x4_i2c_print(1, 9, code);
                }
                else if (character == '*')
                {
                    if(digit_count > 0)
                    {
                        code[--digit_count] = '_';
                        // Update LCD display
                        lcd20x4_i2c_print(1, 9, code);
                    }
                    else
                    {
                        lcd20x4_i2c_clear();
                        lcd20x4_i2c_puts(1, 0, "cancel the operation?");
                        lcd20x4_i2c_puts(2, 0, "* yes   # no");
                        while (1)
                        {
                            if (UART_read(uart4_fd, &rx_buffer, 1) == SUCCESS)
                            {
                                character = convert_to_char(rx_buffer);
                                if (character == '*') // Пользователь выбрал "Да"
                                {
                                    lcd20x4_i2c_clear();
                                    return CANCEL;
                                }
                                else if (character == '#') // Пользователь выбрал "Нет"
                                {
                                    lcd20x4_i2c_clear();
                                    beginDisplay();
                                    lcd20x4_i2c_print(1, 9, code);
                                    break;
                                }
                            }
                            usleep(100000);
                        }
                        lcd20x4_i2c_clear();
                    }
                }
                else if (character == '#')
                {
                    if (digit_count == MAX_LENGTH_ID)
                    {
                        lcd20x4_i2c_clear();
                        code[digit_count] = '\0';
                        int id=atoi(code);
                        return id;
                    }
                    else
                    {
                        lcd20x4_i2c_puts(0, 0, "Invalid ID length. Please enter the code again.");
                        attempts++;
                        beginDisplay();
                    }
                    digit_count = 0; // Reset the digit counter
                }
            }
            else
                return ERROR;

            usleep(100000);
            fflush(stdout);
        }
    }
}
/**
 * @brief Converts a keypad value to its corresponding character.
 *
 * @param value The keypad value.
 * @return char The corresponding character or '\0' if invalid.
 */
char convert_to_char(uint8_t value)
{
    switch (value)
    {
    case 0xE1:
        return '1';
    case 0xE2:
        return '2';
    case 0xE3:
        return '3';
    case 0xE4:
        return '4';
    case 0xE5:
        return '5';
    case 0xE6:
        return '6';
    case 0xE7:
        return '7';
    case 0xE8:
        return '8';
    case 0xE9:
        return '9';
    case 0xEA:
        return '*';
    case 0xEB:
        return '0';
    case 0xEC:
        return '#';
    default:
        return '\0';  // Return null character for invalid input
    }
}
