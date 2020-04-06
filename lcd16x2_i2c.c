#include "lcd16x2_i2c.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#define RETRY_DELAY 10

/////////////////////////////////////////////////////
// in this code, all HAL_Delay are registered by the LCD protocol

static I2C_HandleTypeDef* lcd16x2_i2cHandle;
static uint8_t LCD_I2C_SLAVE_ADDRESS=0;
HAL_StatusTypeDef status;
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
  I2C_transmit_with_check(lcd16x2_i2cHandle, LCD_I2C_SLAVE_ADDRESS, i2cData, 4);
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
  I2C_transmit_with_check(lcd16x2_i2cHandle, LCD_I2C_SLAVE_ADDRESS, i2cData, 4);
}


/**
 * @brief Initialise LCD16x2
 * @param[in] *pI2cHandle - pointer to HAL I2C handle
 */
bool lcd16x2_i2c_init(I2C_HandleTypeDef *pI2cHandle)
{
  HAL_Delay(50);
  lcd16x2_i2cHandle = pI2cHandle;
  if(HAL_I2C_IsDeviceReady(lcd16x2_i2cHandle, LCD_I2C_SLAVE_ADDRESS_0, 5, 500) != HAL_OK)
  {
    if(HAL_I2C_IsDeviceReady(lcd16x2_i2cHandle, LCD_I2C_SLAVE_ADDRESS_1, 5, 500) != HAL_OK)
    {
    	return false;
    }
    else
      LCD_I2C_SLAVE_ADDRESS = LCD_I2C_SLAVE_ADDRESS_1;
  }
  else
    LCD_I2C_SLAVE_ADDRESS = LCD_I2C_SLAVE_ADDRESS_0;
  //Initialise LCD for 4-bit operation
  //1. Wait at least 15ms
  HAL_Delay(45);
  //2. Attentions sequence
  lcd16x2_i2c_sendCommand(0x30);
  HAL_Delay(5);
  lcd16x2_i2c_sendCommand(0x30);
  HAL_Delay(1);
  lcd16x2_i2c_sendCommand(0x30);
  HAL_Delay(8);
  lcd16x2_i2c_sendCommand(0x20);
  HAL_Delay(8);

  lcd16x2_i2c_sendCommand(LCD_FUNCTIONSET | LCD_FUNCTION_N);
  HAL_Delay(1);
  lcd16x2_i2c_sendCommand(LCD_DISPLAYCONTROL);
  HAL_Delay(1);
  lcd16x2_i2c_sendCommand(LCD_CLEARDISPLAY);
  HAL_Delay(3);
  lcd16x2_i2c_sendCommand(0x04 | LCD_ENTRY_ID);
  HAL_Delay(1);
  lcd16x2_i2c_sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAY_D);
  HAL_Delay(3);

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
  HAL_Delay(3);
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
