#include "../Inc/keypad.h"

int rowPins[NUM_ROWS] = {GPIO_PIN_ROW1, GPIO_PIN_ROW2, GPIO_PIN_ROW3, GPIO_PIN_ROW4};
int colPins[NUM_COLS] = {GPIO_PIN_COL1, GPIO_PIN_COL2, GPIO_PIN_COL3};

char keys[NUM_ROWS][NUM_COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

// Define the buffer for the entered ID
char code[MAX_LENGTH_ID + 1] = {'_', '_', '_', '\0'};

Status_t keypad_init()
{
    for (int i = 0; i < NUM_ROWS; i++)
    {
        if (GPIO_init(rowPins[i], "in") == FAILED)
            return FAILED;
    }
    for (int j = 0; j < NUM_COLS; j++)
    {
        if (GPIO_init(colPins[j], "out") == FAILED)
            return FAILED;
    }
    return SUCCESS;
}
/**
 * @brief Initializes and displays the initial keypad entry screen.
 */
void beginDisplay()
{
    printf("beginDisplay\n");
    lcd16x2_i2c_clear();
    // Заполнение массива code символами '_'
    for (int i = 0; i < MAX_LENGTH_ID; i++)
    {
        code[i] = '_';
    }
    // Добавление нулевого символа в конец строки для обозначения конца
    code[MAX_LENGTH_ID] = '\0';
    lcd16x2_i2c_print(0, 0, "Enter ID:");
    lcd16x2_i2c_print(0, 10, code);
    lcd16x2_i2c_print(1, 0, "* DEL/EXIT # OK");
}
/**
 * @brief Checks if the given time duration has passed.
 *
 * @param start_time The starting time of the operation.
 * @param timeout_seconds The timeout duration in seconds.
 * @return int Returns 1 if the timeout has passed, 0 otherwise.
 */
int check_timeout(struct timespec start_time, int timeout_seconds)
{
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    long elapsed_time = (current_time.tv_sec - start_time.tv_sec) + (current_time.tv_nsec - start_time.tv_nsec) / 1000000000;
    return (elapsed_time >= timeout_seconds);
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
    printf("enter_ID_keypad\n");
    beginDisplay();
    struct timespec start_time;
    const int max_execution_time = 30; // Maximum time allowed for input in seconds
    struct timespec current_time;
    int digit_count = 0;
    int attempts = 0;

    // Record the start time
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    while (1)
    {
        if (check_timeout(start_time, max_execution_time))
        {
            displayMessage(__func__, "Timeout: time is up");
            return ERROR;
        }
        char character = read_keypad_value();
        // Handle character input
        if (character != '\0') // If the button is pressed
        {
            if (character != '#' && character != '*' && digit_count < MAX_LENGTH_ID)
            {

                code[digit_count] = character;
                digit_count++;
                // Update LCD display
                lcd16x2_i2c_print(0, 10, code);
            }
            else if (character == '*')
            {
                if (digit_count > 0)
                {
                    code[--digit_count] = '_';
                    // Update LCD display
                    lcd16x2_i2c_print(0, 10, code);
                }
                else
                {
                    lcd16x2_i2c_clear();
                    lcd16x2_i2c_puts(0, 0, "cancel the operation?");
                    lcd16x2_i2c_puts(1, 0, "* yes   # no");

                    struct timespec start_time;
                    clock_gettime(CLOCK_MONOTONIC, &start_time); // Записываем текущее время
                    while (1)
                    {
                        if (check_timeout(start_time, max_execution_time))
                        {
                            displayMessage(__func__, "Timeout: time is up");
                            return ERROR;
                        }

                        character = read_keypad_value();
                        if (character == '*') // The user selected "Yes"
                        {
                            lcd16x2_i2c_clear();
                            return CANCEL;
                        }
                        else if (character == '#') // User selected "No"
                        {
                            lcd16x2_i2c_clear();
                            beginDisplay();
                            lcd16x2_i2c_print(0, 10, code);
                            break;
                        }

                        usleep(200000);
                    }
                    lcd16x2_i2c_clear();
                }
            }
            else if (character == '#')
            {
                if (digit_count == MAX_LENGTH_ID)
                {
                    lcd16x2_i2c_clear();
                    code[digit_count] = '\0';
                    int id = atoi(code);
                    return id;
                }
                else
                {
                    displayMessage(__func__, "Invalid ID, enter again.");
                    attempts++;
                    if (attempts >= g_max_retries)
                    {
                        displayMessage(__func__, "Max attempts reached.");
                        return ERROR;
                    }
                    beginDisplay();
                }
                digit_count = 0; // Reset the digit counter
            }
        }
        usleep(100000); 
        fflush(stdout);
    }
}

char read_keypad_value()
{
    for (int j = 0; j < NUM_COLS; j++)
    {
        int fd = GPIO_open(colPins[j], O_WRONLY);
        if (fd < 0)
        {
            LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Failed to set column high", strerror(errno));
            return '\0';
        }
        GPIO_write(fd, HIGH); // Установка колонки HIGH
        GPIO_close(fd);
    }
    // Проверка каждой колонки
    for (int j = 0; j < NUM_COLS; j++)
    {
        // Установка текущей колонки LOW
        int fd = GPIO_open(colPins[j], O_WRONLY);
        if (fd < 0)
        {
            LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Failed to set column low", strerror(errno));
            return '\0';
        }
        GPIO_write(fd, LOW); // Установка колонки LOW
        GPIO_close(fd);

        // Чтение значений строк
        for (int i = 0; i < NUM_ROWS; i++)
        {
            fd = GPIO_open(rowPins[i], O_RDONLY);
            if (fd < 0)
            {
                LOG_MESSAGE(LOG_ERR, __func__, "strerror", "Failed to read row value", strerror(errno));
                return '\0';
            }
            int value = GPIO_read(fd);
            GPIO_close(fd);
            if (value == 0)
            {
                printf("%c\n",keys[i][j]);
                return keys[i][j];
            }
        }
    }
    return '\0';
}
