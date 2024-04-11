#include "../Inc/lcd20x4_i2c.h"

#define RETRY_DELAY 10
/////////////////////////////////////////////////////

int currentX = 0, currentY = 0;
/* Private functions */
void lcd20x4_i2c_sendCommand(uint8_t command)
{
  const uint8_t command_0_3 = (0xF0 & (command << 4));
  const uint8_t command_4_7 = (0xF0 & command);
  uint8_t i2cData[4] =
      {
          command_4_7 | LCD_EN | LCD_BK_LIGHT,
          command_4_7 | LCD_BK_LIGHT,
          command_0_3 | LCD_EN | LCD_BK_LIGHT,
          command_0_3 | LCD_BK_LIGHT,
      };
  I2C_write(i2cData, 4);
  usleep(50);
}

void lcd20x4_i2c_sendData(uint8_t data)
{
  const uint8_t data_0_3 = (0xF0 & (data << 4));
  const uint8_t data_4_7 = (0xF0 & data);
  uint8_t i2cData[4] =
      {
          data_4_7 | LCD_EN | LCD_BK_LIGHT | LCD_RS,
          data_4_7 | LCD_BK_LIGHT | LCD_RS,
          data_0_3 | LCD_EN | LCD_BK_LIGHT | LCD_RS,
          data_0_3 | LCD_BK_LIGHT | LCD_RS,
      };
  I2C_write(i2cData, 4);
  usleep(20);
}

/**
 * @brief Initialise LCD20x4
 * @param[in] *pI2cHandle - pointer to HAL I2C handle
 */
bool lcd20x4_i2c_init()
{
  usleep(50);
  if (I2C_Init() != 1)
    return false;
  // Initialise LCD for 4-bit operation
  // 1. Wait at least 15ms
  usleep(45);
  // 2. Attentions sequence
  lcd20x4_i2c_sendCommand(0x30);
  usleep(5);
  lcd20x4_i2c_sendCommand(0x30);
  usleep(1);
  lcd20x4_i2c_sendCommand(0x30);
  usleep(8);
  lcd20x4_i2c_sendCommand(0x20);
  usleep(8);

  lcd20x4_i2c_sendCommand(LCD_FUNCTIONSET | LCD_FUNCTION_N);
  usleep(1);
  lcd20x4_i2c_sendCommand(LCD_DISPLAYCONTROL);
  usleep(1);
  lcd20x4_i2c_sendCommand(LCD_CLEARDISPLAY);
  usleep(3);
  lcd20x4_i2c_sendCommand(0x04 | LCD_ENTRY_ID);
  usleep(1);
  lcd20x4_i2c_sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAY_D);
  usleep(3);

  return true;
}

/**
 * @brief Set cursor position
 * @param[in] row - 0 or 3 for line1 or line2 or ...
 * @param[in] col - 0 - 19 (20columns LCD)
 */
void lcd20x4_i2c_setCursor(uint8_t row, uint8_t col)
{
  if (row >= 4)
    row = 0; // Если row больше 3, установим его в 0
  if (col >= 20)
    col = 0;
  // Адресация строк 20x4 дисплея:
  const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
  uint8_t maskData = (col & 0x0F) | row_offsets[row];
  lcd20x4_i2c_sendCommand(0x80 | maskData);
}
// Move to beginning of 1st line
void lcd20x4_i2c_1stLine(void)
{
  lcd20x4_i2c_setCursor(0, 0);
}
// Move to beginning of 2nd line
void lcd20x4_i2c_2ndLine(void)
{
  lcd20x4_i2c_setCursor(1, 0);
}
// Move to beginning of 3rd line
void lcd20x4_i2c_3rdLine(void)
{
  lcd20x4_i2c_setCursor(2, 0);
}
// Move to beginning of 4th line
void lcd20x4_i2c_4thLine(void)
{
  lcd20x4_i2c_setCursor(3, 0);
}
// Display clear
void lcd20x4_i2c_clear(void)
{
  lcd20x4_i2c_sendCommand(LCD_CLEARDISPLAY);
  usleep(5);
}
// Display ON/OFF, to hide all characters, but not clear
void lcd20x4_i2c_display(bool state)
{
  if (state)
  {
    lcd20x4_i2c_sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAY_B | LCD_DISPLAY_C | LCD_DISPLAY_D);
  }
  else
  {
    lcd20x4_i2c_sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAY_B | LCD_DISPLAY_C);
  }
}
// Print to display
void lcd20x4_i2c_printf(const char *str, ...)
{
  char stringArray[_LCD_COLS];
  va_list args;
  va_start(args, str);
  vsprintf(stringArray, str, args);
  va_end(args);
  for (uint8_t i = 0; i < strlen(stringArray) && i < _LCD_COLS; i++)
  {
    lcd20x4_i2c_sendData((uint8_t)stringArray[i]);
  }
}

// Функция для определения длины слова (до первого пробела)
int wordLength(const char *str)
{
  int length = 0;
  while (*str && *str != ' ' && *str != '\t' && *str != '\n' && *str != '\r')
  {
    length++;
    str++;
  }
  return length;
}
void lcd20x4_i2c_puts(uint8_t x, uint8_t y, const char *str)
{
  lcd20x4_i2c_clear();
  usleep(1000);
  lcd20x4_i2c_setCursor(x, y);
  usleep(1000);
  currentX = x;
  currentY = y;
  uint8_t wordStartX = currentX; // Переменная для отслеживания начала текущего слова

  for (int i = 0; i < strlen(str); i++)
  {
    if (str[i] == ' ' || str[i] == '\t')
    {
      lcd20x4_i2c_sendData((uint8_t)(str[i]));
      if (currentY < _LCD_COLS)
        currentY++;
      else
      {
        currentY = 0;
        currentX++;
        lcd20x4_i2c_setCursor(currentX, currentY);
        usleep(10);
      }
      wordStartX = currentY;
    }
    else
    { 
      int wordLen = wordLength(&str[i]);
      if ((currentY + wordLen) >= _LCD_COLS)
      {
        currentY = 0;
        wordStartX = 0;
        currentX++;
        lcd20x4_i2c_setCursor(currentX, currentY);
        usleep(10);
      }
      //
      for (int j = 0; j < wordLen; j++)
      {
        lcd20x4_i2c_sendData((uint8_t)(str[i + j]));
        currentY++;
      }
      i += wordLen - 1;
    }
  }
}
