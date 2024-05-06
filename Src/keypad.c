#include "../Inc/keypad.h"
// Function to enter and validate an ID from keypad

extern int uart4_fd;

int enter_ID_keypad()
{
    char code[MAX_LENGTH_ID + 1] = {'_', '_', '_', '\0'};
    lcd20x4_i2c_print(0, 0, "Enter ID: ");
    lcd20x4_i2c_print(1, 9, code);
    lcd20x4_i2c_print(3, 0, "* delete, # confirm");
    uint8_t rx_buffer;
    int digit_count = 0;
    int attempts = 0;
    // we'll wait half a minute
    clock_t start_time = clock();
    const clock_t max_execution_time = 30 * CLOCKS_PER_SEC;
    while ((clock() - start_time) <= max_execution_time && attempts < MAX_RETRIES)
    {
        if(UART_read(uart4_fd, &rx_buffer, 1)>0)
        //if (read(uart4_fd, &rx_buffer, 1) > 0)
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
                else if (character == '*' && digit_count > 0)
                {
                    code[digit_count] = '_';
                    // Update LCD display
                    lcd20x4_i2c_print(1, 9, code);
                    digit_count--; // Delete the last entered digit
                }
                else if (character == '#')
                {
                    if (digit_count == MAX_LENGTH_ID)
                    {
                        code[digit_count] = '\0';
                        return atoi(code);
                    }
                    else
                    {
                        lcd20x4_i2c_puts(0, 0, "Invalid ID length. Please enter the code again.");
                        attempts++;
                    }
                    digit_count = 0; // Reset the digit counter
                }
            }
            else
                return -1;

            usleep(100000);
            fflush(stdout);
        }
        if ((clock() - start_time) > max_execution_time)
        {
            lcd20x4_i2c_puts(0, 0, "Timeout: One minute has passed.");
            return -1;
        }
    }
}

char convert_to_char(uint8_t value)
{
    switch (value)
    {
    case 0xE1:
        printf("1 ");
        return '1';
    case 0xE2:
        printf("2 ");
        return '2';
    case 0xE3:
        printf("3 ");
        return '3';
    case 0xE4:
        printf("4 ");
        return '4';
    case 0xE5:
        printf("5 ");
        return '5';
    case 0xE6:
        printf("6 ");
        return '6';
    case 0xE7:
        printf("7 ");
        return '7';
    case 0xE8:
        printf("8 ");
        return '8';
    case 0xE9:
        printf("9 ");
        return '9';
    case 0xEA:
        printf("* ");
        return '*';
    case 0xEB:
        printf("0 ");
        return '0';
    case 0xEC:
        printf("# ");
        return '#';
    default:
        return '\0'; // Возвращаем null-символ в случае неправильного ввода
    }
}
