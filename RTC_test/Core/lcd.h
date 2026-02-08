/**
  ******************************************************************************
  * @file			lcd.h
  * @brief		Заголовочный файл модуля работы с LCD 1602 по шине I2C (PCF8574)
  ******************************************************************************
  */

#ifndef LCD_H
#define LCD_H

#include "stm32f10x.h"
#include "rtc.h"
#include "i2c.h"
#include "delay.h"
#include <stdio.h>

static void lcdSendNibble(uint8_t data, uint8_t rs);
static void lcdWrite4Bits(uint8_t data, uint8_t rs);

/* Адрес LCD (PCF8574) на шине I2C*/
#define LCD_ADDRESS	0x27

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

// Биты управления (схема подключения PCF8574T к LCD)
#define LCD_RS_PIN 0x01  // P0: Register Select (1 - данные, 0 - команда)
#define LCD_RW_PIN 0x02  // P1: Read/Write (1 - чтение, 0 - запись)
#define LCD_E_PIN  0x04  // P2: Вход тактовых импульсов (строб)
#define LCD_BL_PIN 0x08  // P3: Подсветка (1 - вкл, 0 - выкл)
#define LCD_D4_PIN 0x10  // P4: Data bit 4
#define LCD_D5_PIN 0x20  // P5: Data bit 5
#define LCD_D6_PIN 0x40  // P6: Data bit 6
#define LCD_D7_PIN 0x80  // P7: Data bit 7


void lcdInit(void);												// Инициализация LCD
//void lcdSendCommand(uint8_t);						// Отправка команды на LCD
//void lcdSend_Data(uint8_t);							// Отправка данных на LCD
void lcdSetCursor(uint8_t, uint8_t);			// Установка позиции курсора
//void lcdPrintChar(char c);							// Вывод символа
void lcdPrintString(const char*);					// Вывод строки
void lcdClear(void);											// Очистка дисплея
// Обновление времени на дисплее
void lcdUpdateTime(RTCTimeDate*);

#endif /* LCD_H */
