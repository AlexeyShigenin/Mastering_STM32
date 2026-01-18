/**
  ******************************************************************************
  * @file    lcd.h
  * @brief   Заголовочный файл для модуля LCD
  ******************************************************************************
  */

#ifndef __LCD_H
#define __LCD_H

#include <stdint.h>
#include "rtc.h"

/* Команды HD44780 */
#define LCD_CLEAR_DISPLAY    0x01
#define LCD_RETURN_HOME      0x02
#define LCD_ENTRY_MODE_SET   0x04
#define LCD_DISPLAY_CONTROL  0x08
#define LCD_CURSOR_SHIFT     0x10
#define LCD_FUNCTION_SET     0x20
#define LCD_SET_CGRAM_ADDR   0x40
#define LCD_SET_DDRAM_ADDR   0x80

/* Флаги для режима ввода */
#define LCD_ENTRY_RIGHT      0x00
#define LCD_ENTRY_LEFT       0x02
#define LCD_ENTRY_SHIFT_ON   0x01
#define LCD_ENTRY_SHIFT_OFF  0x00

/* Флаги управления дисплеем */
#define LCD_DISPLAY_ON       0x04
#define LCD_DISPLAY_OFF      0x00
#define LCD_CURSOR_ON        0x02
#define LCD_CURSOR_OFF       0x00
#define LCD_BLINK_ON         0x01
#define LCD_BLINK_OFF        0x00

/* Флаги сдвига */
#define LCD_SHIFT_DISPLAY    0x08
#define LCD_SHIFT_CURSOR     0x00
#define LCD_SHIFT_RIGHT      0x04
#define LCD_SHIFT_LEFT       0x00

/* Флаги функции */
#define LCD_8BIT_MODE        0x10
#define LCD_4BIT_MODE        0x00
#define LCD_2LINE            0x08
#define LCD_1LINE            0x00
#define LCD_5x10_DOTS        0x04
#define LCD_5x8_DOTS         0x00

/* Прототипы функций */
void lcd_init(void);
void lcd_send_command(uint8_t cmd);
void lcd_send_data(uint8_t data);
void lcd_clear(void);
void lcd_set_cursor(uint8_t row, uint8_t col);
void lcd_print_char(char c);
void lcd_print_string(const char* str);
void lcd_update_time(RTC_TimeTypeDef* time, RTC_DateTypeDef* date);

#endif /* __LCD_H */
