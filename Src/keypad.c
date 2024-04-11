#include "../Inc/keypad.h"
int uart_fd;

int receive_ID_keypad()
{
    char code[MAX_LENGTH_ID + 1];
    uint8_t rx_buffer;
    int digit_count = 0;
    int attempts = 0;
    clock_t start_time = clock();
	const clock_t max_execution_time = 30 * CLOCKS_PER_SEC;
    while ((clock() - start_time) <= max_execution_time)
    {
        if (read(uart_fd, &rx_buffer, 1) > 0)
        {
            char character = convert_to_char(rx_buffer);
            if (character != '\0' && character != '#')
            {
                if (character == '*' && digit_count > 0)
                        digit_count--; // Удаляем последнюю введенную цифру
                if (digit_count <  MAX_LENGTH_ID && character != '*')
                    code[digit_count++] = character;
                showMessageOnDisplay(code,1,0);
            }
            else if (character == '#')
            {
                code[digit_count] = '\0'; // Добавляем нуль-терминатор для корректной обработки строки
                if (strlen(code) == MAX_LENGTH_ID)
                    return atoi(code);
                else
                {
                    showMessageOnDisplay("Invalid ID length. Please enter the code again.",1,0);
                    //хочу вызвать функцию повторно. максимальное количество раз 2
                }
                digit_count = 0; // Сбрасываем счетчик цифр
            }
        }
        else
            return -1;

        usleep(100000);
        fflush(stdout);
    }
    if ((clock() - start_time) > max_execution_time) 
    {
        showMessageOnDisplay("Timeout: One minute has passed.",1,0);
        return -1;
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
