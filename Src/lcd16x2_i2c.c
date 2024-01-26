#include "../Inc/lcd16x2_i2c.h"

#define RETRY_DELAY 10
/////////////////////////////////////////////////////
// in this code, all HAL_Delay are registered by the LCD protocol

/* Private functions */
void lcd16x2_i2c_sendCommand(uint8_t command)
{
  const uint8_t command_0_3 = (0xF0 & (command<<4));
  const uint8_t command_4_7 = (0xF0 & command);
  uint8_t i2cData[4] =
  {
      command_4_7 | LCD_EN | LCD_BK_LIGHT,
      command_4_7 | LCD_BK_LIGHT,
      command_0_3 | LCD_EN | LCD_BK_LIGHT,
      command_0_3 | LCD_BK_LIGHT,
  };
  I2C_write(i2cData, 4);
}

void lcd16x2_i2c_sendData(uint8_t data)
{
  const uint8_t data_0_3 = (0xF0 & (data<<4));
  const uint8_t data_4_7 = (0xF0 & data);
  uint8_t i2cData[4] =
  {
      data_4_7 | LCD_EN | LCD_BK_LIGHT | LCD_RS,
      data_4_7 | LCD_BK_LIGHT | LCD_RS,
      data_0_3 | LCD_EN | LCD_BK_LIGHT | LCD_RS,
      data_0_3 | LCD_BK_LIGHT | LCD_RS,
  };
  I2C_write(i2cData, 4);
}


/**
 * @brief Initialise LCD16x2
 * @param[in] *pI2cHandle - pointer to HAL I2C handle
 */
bool lcd16x2_i2c_init()
{
  printf("lcd16x2_i2c_init()\r\n");
  usleep (50);
  if(I2C_Init()!=1) return false;
  //Initialise LCD for 4-bit operation
  //1. Wait at least 15ms
  usleep (45);
  //2. Attentions sequence
  lcd16x2_i2c_sendCommand(0x30);
  usleep (5);
  lcd16x2_i2c_sendCommand(0x30);
  usleep (1);
  lcd16x2_i2c_sendCommand(0x30);
  usleep (8);
  lcd16x2_i2c_sendCommand(0x20);
  usleep (8);

  lcd16x2_i2c_sendCommand(LCD_FUNCTIONSET | LCD_FUNCTION_N);
  usleep (1);
  lcd16x2_i2c_sendCommand(LCD_DISPLAYCONTROL);
  usleep (1);
  lcd16x2_i2c_sendCommand(LCD_CLEARDISPLAY);
  usleep (3);
  lcd16x2_i2c_sendCommand(0x04 | LCD_ENTRY_ID);
  usleep (1);
  lcd16x2_i2c_sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAY_D);
  usleep (3);

  return true;
}

/**
 * @brief Set cursor position
 * @param[in] row - 0 or 3 for line1 or line2 or ...
 * @param[in] col - 0 - 19 (20columns LCD)
 */
void lcd16x2_i2c_setCursor(uint8_t row, uint8_t col)
{
	//int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if (row > 3)
			row = 0;
  uint8_t maskData;
  //maskData=0x80 |(col + row_offsets[row]);

  maskData = (col)&0x0F;
  switch(row)
  {
  case 0:
	  maskData |= 0x80;
	  break;
  case 1:
	  maskData |= 0xC0;
	  break;
  case 2:
	  maskData |= 0x80;
	  break;
  case 3:
	  maskData |= 0xC0;
	  break;
  }
  lcd16x2_i2c_sendCommand(maskData);
}
//Move to beginning of 1st line
void lcd16x2_i2c_1stLine(void)
{
  lcd16x2_i2c_setCursor(0,0);
}
// Move to beginning of 2nd line
void lcd16x2_i2c_2ndLine(void)
{
  lcd16x2_i2c_setCursor(1,0);
}
// Move to beginning of 3rd line
void lcd16x2_i2c_3rdLine(void)
{
  lcd16x2_i2c_setCursor(2,0);
}
// Move to beginning of 4th line
void lcd16x2_i2c_4thLine(void)
{
  lcd16x2_i2c_setCursor(3,0);
}
// Display clear
void lcd16x2_i2c_clear(void)
{
  lcd16x2_i2c_sendCommand(LCD_CLEARDISPLAY);
  usleep (3);
}
//Display ON/OFF, to hide all characters, but not clear
void lcd16x2_i2c_display(bool state)
{
  if(state)
  {
    lcd16x2_i2c_sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAY_B | LCD_DISPLAY_C | LCD_DISPLAY_D);
  }
  else
  {
    lcd16x2_i2c_sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAY_B | LCD_DISPLAY_C);
  }
}
//Print to display
void lcd16x2_i2c_printf(const char* str, ...)
{
  char stringArray[30];
  va_list args;
  va_start(args, str);
  vsprintf(stringArray, str, args);
  va_end(args);
  for(uint8_t i=0;  i<strlen(stringArray) && i<20; i++)
  {
    lcd16x2_i2c_sendData((uint8_t)stringArray[i]);
  }
}
