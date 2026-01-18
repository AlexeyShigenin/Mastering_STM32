/**
  ******************************************************************************
  * @file    lcd.c
  * @brief   Модуль работы с LCD 1602 через PCF8574T
  * 
  * LCD подключен через I2C расширитель портов PCF8574T.
  * Используется 4-битный режим передачи данных.
  * Дисплей отображает текущее время и состояние системы.
  ******************************************************************************
  */

#include "lcd.h"
#include "i2c.h"
#include "delay.h"
#include <string.h>

/* Биты управления PCF8574T */
#define LCD_RS_PIN 0x01  // P0: Register Select (1 - данные, 0 - команда)
#define LCD_RW_PIN 0x02  // P1: Read/Write (1 - чтение, 0 - запись)
#define LCD_E_PIN  0x04  // P2: Enable
#define LCD_BL_PIN 0x08  // P3: Подсветка (1 - вкл, 0 - выкл)
#define LCD_D4_PIN 0x10  // P4: Data bit 4
#define LCD_D5_PIN 0x20  // P5: Data bit 5
#define LCD_D6_PIN 0x40  // P6: Data bit 6
#define LCD_D7_PIN 0x80  // P7: Data bit 7

/* Статические функции */
static void lcd_send_nibble(uint8_t data, uint8_t rs);
static void lcd_write_4bits(uint8_t data, uint8_t rs);

/**
  * @brief  Инициализация LCD
  * @param  None
  * @retval None
  */
void lcd_init(void) {
    // Задержка для стабилизации питания LCD
    delay_ms(50);
    
    // Начальная последовательность инициализации (4-битный режим)
    lcd_send_nibble(0x30, 0);  // Функция установки 8-бит
    delay_ms(5);
    lcd_send_nibble(0x30, 0);  // Повтор
    delay_ms(1);
    lcd_send_nibble(0x30, 0);  // Еще раз
    delay_ms(1);
    
    // Переход в 4-битный режим
    lcd_send_nibble(0x20, 0);
    delay_ms(1);
    
    // Установка режима: 2 строки, 5x8 точек
    lcd_send_command(LCD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2LINE | LCD_5x8_DOTS);
    
    // Выключение дисплея
    lcd_send_command(LCD_DISPLAY_CONTROL | LCD_DISPLAY_OFF);
    
    // Очистка дисплея
    lcd_clear();
    
    // Установка режима ввода
    lcd_send_command(LCD_ENTRY_MODE_SET | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_OFF);
    
    // Включение дисплея с курсором
    lcd_send_command(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);
    
    // Включение подсветки
    i2c_write_byte(PCF8574_ADDRESS, LCD_BL_PIN);
}

/**
  * @brief  Отправка ниббла (4 бита) на LCD
  * @param  data: данные (нижние 4 бита)
  * @param  rs: флаг RS (0 - команда, 1 - данные)
  * @retval None
  */
static void lcd_send_nibble(uint8_t data, uint8_t rs) {
    uint8_t nibble = data & 0x0F;
    uint8_t control = 0;
    
    // Установка битов данных
    if (nibble & 0x01) control |= LCD_D4_PIN;
    if (nibble & 0x02) control |= LCD_D5_PIN;
    if (nibble & 0x04) control |= LCD_D6_PIN;
    if (nibble & 0x08) control |= LCD_D7_PIN;
    
    // Установка бита RS
    if (rs) control |= LCD_RS_PIN;
    
    // Включение подсветки
    control |= LCD_BL_PIN;
    
    // Установка бита E и отправка
    i2c_write_byte(PCF8574_ADDRESS, control | LCD_E_PIN);
    delay_ms(1);
    i2c_write_byte(PCF8574_ADDRESS, control);
    delay_ms(1);
}

/**
  * @brief  Запись байта в 4-битном режиме
  * @param  data: данные для записи
  * @param  rs: флаг RS
  * @retval None
  */
static void lcd_write_4bits(uint8_t data, uint8_t rs) {
    // Отправка старшего ниббла
    lcd_send_nibble(data >> 4, rs);
    // Отправка младшего ниббла
    lcd_send_nibble(data & 0x0F, rs);
}

/**
  * @brief  Отправка команды на LCD
  * @param  cmd: команда
  * @retval None
  */
void lcd_send_command(uint8_t cmd) {
    lcd_write_4bits(cmd, 0);
    if (cmd == LCD_CLEAR_DISPLAY || cmd == LCD_RETURN_HOME) {
        delay_ms(2);
    } else {
        delay_ms(1);
    }
}

/**
  * @brief  Отправка данных на LCD
  * @param  data: данные
  * @retval None
  */
void lcd_send_data(uint8_t data) {
    lcd_write_4bits(data, 1);
    delay_ms(1);
}

/**
  * @brief  Очистка дисплея
  * @param  None
  * @retval None
  */
void lcd_clear(void) {
    lcd_send_command(LCD_CLEAR_DISPLAY);
    delay_ms(2);
}

/**
  * @brief  Установка позиции курсора
  * @param  row: строка (0 или 1)
  * @param  col: столбец (0-15)
  * @retval None
  */
void lcd_set_cursor(uint8_t row, uint8_t col) {
    uint8_t address;
    
    // Адреса начала строк в DDRAM
    if (row == 0) {
        address = 0x00;
    } else {
        address = 0x40;
    }
    
    address += col;
    
    // Отправка команды установки адреса
    lcd_send_command(LCD_SET_DDRAM_ADDR | address);
}

/**
  * @brief  Вывод символа
  * @param  c: символ
  * @retval None
  */
void lcd_print_char(char c) {
    lcd_send_data(c);
}

/**
  * @brief  Вывод строки
  * @param  str: указатель на строку
  * @retval None
  */
void lcd_print_string(const char* str) {
    while (*str) {
        lcd_print_char(*str++);
    }
}

/**
  * @brief  Обновление времени на дисплее
  * @param  time: указатель на структуру времени
  * @param  date: указатель на структуру даты
  * @retval None
  */
void lcd_update_time(RTC_TimeTypeDef* time, RTC_DateTypeDef* date) {
    char buffer[17];
    
    // Первая строка: дата
    lcd_set_cursor(0, 0);
    sprintf(buffer, "Date:%02d/%02d/20%02d", 
            date->day, date->month, date->year);
    lcd_print_string(buffer);
    
    // Вторая строка: время
    lcd_set_cursor(1, 0);
    sprintf(buffer, "Time:%02d:%02d:%02d", 
            time->hours, time->minutes, time->seconds);
    lcd_print_string(buffer);
}
