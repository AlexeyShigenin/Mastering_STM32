/**
  ******************************************************************************
  * @file			lcd.h
  * @brief		Заголовочный файл модуля работы с LCD 1602 по шине I2C (PCF8574)
	* @author		Алексей Шигенин
  ******************************************************************************
  */

#ifndef LCD_H
#define LCD_H

#include "stm32f10x.h"

void lcd_init(void);												// Инициализация LCD
void lcd_send_command(uint8_t);							// Отправка команды на LCD
void lcd_send_data(uint8_t);								// Отправка данных на LCD
void lcd_set_cursor(uint8_t, uint8_t);			// Установка позиции курсора
void lcd_print_char(char c);								// Вывод символа
void lcd_print_string(const char*);					// Вывод строки
void lcd_clear(void);												// Очистка дисплея
// Обновление времени на дисплее
//void lcd_update_time(RTC_TimeTypeDef*, RTC_DateTypeDef*);

#endif /* LCD_H */
