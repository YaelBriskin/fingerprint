#ifndef LCD20x4_I2C_H_
#define LCD20x4_I2C_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include "../Inc/I2C.h"


bool lcd20x4_i2c_init();
void lcd20x4_i2c_setCursor(uint8_t row, uint8_t col);
void lcd20x4_i2c_1stLine(void);
void lcd20x4_i2c_2ndLine(void);
void lcd20x4_i2c_3rdLine(void);
void lcd20x4_i2c_4thLine(void);
void lcd20x4_i2c_TwoLines(void);
void lcd20x4_i2c_OneLine(void);
void lcd20x4_i2c_cursorShow(bool state);
void lcd20x4_i2c_clear(void);
void lcd20x4_i2c_sendCommand(uint8_t command);
void lcd20x4_i2c_sendData(uint8_t data);
void lcd20x4_i2c_display(bool state);
void lcd20x4_i2c_shiftRight(uint8_t offset);
void lcd20x4_i2c_shiftLeft(uint8_t offset);
void lcd20x4_i2c_printf(const char* str, ...);
void lcd20x4_i2c_print(uint8_t row, uint8_t col, const char *message);
void lcd20x4_i2c_display(bool state);
void lcd20x4_i2c_shiftRight(uint8_t offset);
void lcd20x4_i2c_shiftLeft(uint8_t offset);
void lcd20x4_i2c_printf(const char* str, ...);
void lcd20x4_i2c_puts(uint8_t x, uint8_t y, const char* str);

/* LCD Commands */
#define LCD_CLEARDISPLAY    0x01
#define LCD_RETURNHOME      0x02
#define LCD_ENTRYMODESET    0x04
#define LCD_DISPLAYCONTROL  0x08
#define LCD_CURSORSHIFT     0x10
#define DISPLAY_SHIFT       0x18
#define LCD_FUNCTIONSET     0x20
#define LCD_SETCGRAMADDR    0x40
#define LCD_SETDDRAMADDR    0x80

/* Commands bitfields */
//1) Entry mode Bitfields
#define LCD_ENTRY_SH      0x01
#define LCD_ENTRY_ID      0x02
//2) Display control
#define LCD_DISPLAY_B     0x01
#define LCD_DISPLAY_C     0x02
#define LCD_DISPLAY_D     0x04
//3) Shift control
#define LCD_SHIFT_RL      0x04
#define LCD_SHIFT_SC      0x08
//4) Function set control
#define LCD_FUNCTION_F    0x04
#define LCD_FUNCTION_N    0x08
#define LCD_FUNCTION_DL   0x10

/* I2C Control bits */
#define LCD_RS        (1 << 0)
#define LCD_RW        (1 << 1)
#define LCD_EN        (1 << 2)
#define LCD_BK_LIGHT  (1 << 3)

/* Library variables */

//#define LCD_I2C_SLAVE_ADDRESS_0  0x4E
//#define LCD_I2C_SLAVE_ADDRESS_1  0x7E

#define _LCD_COLS         20
#define _LCD_ROWS         4

//Special characters definitions
#define LCD_ALPHA               0xE0      //𝛼
#define LCD_BETA                0xE2      //β
#define LCD_BLACK_RECTANGLE     0xFF      //◼️
#define LCD_DIVISION_SIGN       0xFD      //÷
#define LCD_EPSILON             0xE3      //ε
#define LCD_INFINITY            0xF3      //∞
#define LCD_LEFT_ARROW          0x7F      //￩
#define LCD_MU                  0xE4      //μ
#define LCD_OMEGA               0xF4      //Ω
#define LCD_PI                  0xF7      //π
#define LCD_RHO                 0xE6      //ρ
#define LCD_RIGHT_ARROW         0x7E      //￫
#define LCD_SIGMA               0xE5      //𝜎
#define LCD_SUMMATION           0xF6      //Σ
#define LCD_THETA               0xF2      //θ
#define LCD_YEN                 0x5C      //¥

#define LCD_ROW0        0x80
#define LCD_ROW1        0xC0
#define LCD_ROW2        0x94
#define LCD_ROW3        0xD4

#endif /* LCD20x4_I2C_H_ */
