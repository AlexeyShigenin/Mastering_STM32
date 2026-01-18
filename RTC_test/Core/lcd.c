/**
	******************************************************************************
	* @file			lcd.c
	* @brief		Функции для работы с LCD 1602 по шине I2C (PCF8574T)
	* @author		Алексей Шигенин	
	*
	* LCD подключен через I2C расширитель портов PCF8574T.
  * Используется 4-битный режим передачи данных.
	******************************************************************************
	*/

#include "lcd.h"
#include "i2c.h"
#include "delay.h"
#include <string.h>

static void lcd_send_nibble(uint8_t data, uint8_t rs);
static void lcd_write_4bits(uint8_t data, uint8_t rs);

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

/**
	******************************************************************************
  * @brief  Инициализация LCD
  * @param  None
  * @retval None
	******************************************************************************
	*/
void lcd_init(void) {
	// Задержка для стабилизации питания LCD
	delay_ms(50);
    
	// Начальная последовательность инициализации.
	// Перед включением 4-битного режима необходимо трижды
	// отправить команду установки 8-битного режима LCD_FUNCTION_SET | LCD_8BIT_MODE.
	// После трехкратной отправки команды дисплей выравнивает
	// свой внутренний счетчик и начинает правильно интерпретировать поток.
	// Т.к. LCD_FUNCTION_SET | LCD_8BIT_MODE это 0x30 (0011 0000), а 
	// send_nibble отправляет только 4 младшие бита, то необходимо сдвинуть данные
	// на 4 разряда вправо, т.е. нужно отправить 0000 0011 (0x03)
	lcd_send_nibble(0x03, 0);  // 1
	delay_ms(6);
	lcd_send_nibble(0x03, 0);  // 2
	delay_ms(6);
	lcd_send_nibble(0x03, 0);  // 3
	delay_ms(6);

	// Переход в 4-битный режим
	// Аналогично команда LCD_FUNCTION_SET | LCD_8BIT_MODE это 0x20 (0010 0000)
	// соответственно, нужно сдвинуть на 4 разряда вправо, 0000 0010 (0x02)
	lcd_send_nibble(0x02, 0);  // Старший ниббл команды 0x20
	delay_ms(1);

	// Теперь можно отправлять команды в 4-х битном режиме.
    
	// Установка режима: 2 строки, 5x8 точек
	lcd_send_command(LCD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2LINE | LCD_5x8_DOTS);
    
	// Выключение дисплея
	lcd_send_command(LCD_DISPLAY_CONTROL | LCD_DISPLAY_OFF);
    
	// Очистка дисплея
	lcd_send_command(LCD_CLEAR_DISPLAY);
    
	// Установка режима ввода
	lcd_send_command(LCD_ENTRY_MODE_SET | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_OFF);
    
	// Включение дисплея с курсором
	lcd_send_command(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);
    
	// Включение подсветки
	i2c_write_byte(LCD_ADDRESS, LCD_BL_PIN);
}

/**
	******************************************************************************
	* @brief  Отправка полубайта (4 бита) на LCD
	* @param  data: данные (нижние 4 бита)
	* @param  rs: флаг RS (0 - команда, 1 - данные)
	* @retval None
	******************************************************************************
	*/
static void lcd_send_nibble(uint8_t data, uint8_t rs) {
    
	// Извлекаем из data только младшие 4 бита
	// data & 0x0F = маска, оставляющая только биты 0-3
	uint8_t nibble = data & 0x0F;
	
	// Инициализируем переменную для формирования байта отправки
	// control будет содержать все управляющие биты для PCF8574T
	uint8_t control = 0;
    
	// Установка битов данных D4-D7 на основе ниббла
	// Проверяем каждый бит ниббла и устанавливаем соответствующий бит в control
	if (nibble & 0x01) control |= LCD_D4_PIN;
	if (nibble & 0x02) control |= LCD_D5_PIN;
	if (nibble & 0x04) control |= LCD_D6_PIN;
	if (nibble & 0x08) control |= LCD_D7_PIN;
    
	// Установка бита RS (Register Select)
	// RS = 0: отправляем команду (запись в регистр команд)
	// RS = 1: отправляем данные (запись в регистр данных)
	if (rs) control |= LCD_RS_PIN;
    
	// LCD_BL_PIN = 1 включает подсветку, 0 - выключает
	// Подсветка всегда включена
	control |= LCD_BL_PIN;
    
	// Последовательность стробирования (strobe)
	// Для записи данных на LCD необходимо:
	// 1. Установить бит E (Enable) в 1
	// 2. Выдержать паузу (не менее 450 нс)
	// 3. Установить бит E в 0
    
	// 1. Сначала выставляем на шину байт с установленным битом E
	// (добавляем бит E к остальным установленным битам)
	i2c_write_byte(LCD_ADDRESS, control | LCD_E_PIN);
	
	// 2. Пауза > 450 нс
	delay_ms(1);
	
	// 3. Отправляем байт без бита E (устанавливаем E в 0)
	// Это создает спад импульса, по которому LCD защелкивает данные
	i2c_write_byte(LCD_ADDRESS, control);
	
	// Пауза для стабилизации
	delay_ms(1);
}

/**
	******************************************************************************
  * @brief  Запись байта в 4-битном режиме
  * @param  data: данные для записи
  * @param  rs: флаг RS
  * @retval None
	******************************************************************************
	*/
static void lcd_write_4bits(uint8_t data, uint8_t rs) {
    // Отправка старшего ниббла
    lcd_send_nibble(data >> 4, rs);
    // Отправка младшего ниббла
    lcd_send_nibble(data & 0x0F, rs);
}

/**
	******************************************************************************
  * @brief  Отправка команды на LCD
  * @param  cmd: команда
  * @retval None
	******************************************************************************
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
	******************************************************************************
  * @brief  Отправка данных на LCD
  * @param  data: данные
  * @retval None
	******************************************************************************
	*/
void lcd_send_data(uint8_t data) {
    lcd_write_4bits(data, 1);
    delay_ms(1);
}

/**
	******************************************************************************
  * @brief  Очистка дисплея
  * @param  None
  * @retval None
	******************************************************************************
	*/
void lcd_clear(void) {
    lcd_send_command(LCD_CLEAR_DISPLAY);
    delay_ms(1);
}

/**
	******************************************************************************
  * @brief  Установка позиции курсора
  * @param  row: строка (0 или 1)
  * @param  col: столбец (0-15)
  * @retval None
	******************************************************************************
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
	******************************************************************************
  * @brief  Вывод символа
  * @param  c: символ
  * @retval None
	******************************************************************************
	*/
void lcd_print_char(char c) {
    lcd_send_data(c);
}

/**
	******************************************************************************
  * @brief  Вывод строки
  * @param  str: указатель на строку
  * @retval None
	******************************************************************************
	*/
void lcd_print_string(const char* str) {
    while (*str) {
        lcd_print_char(*str++);
    }
}

///**
//	******************************************************************************
//  * @brief  Обновление времени на дисплее
//  * @param  time: указатель на структуру времени
//  * @param  date: указатель на структуру даты
//  * @retval None
//	******************************************************************************
//	*/
//void lcd_update_time(RTC_TimeTypeDef* time, RTC_DateTypeDef* date) {
//    char buffer[17];
//    
//    // Первая строка: дата
//    lcd_set_cursor(0, 0);
//    sprintf(buffer, "Date:%02d/%02d/20%02d", 
//            date->day, date->month, date->year);
//    lcd_print_string(buffer);
//    
//    // Вторая строка: время
//    lcd_set_cursor(1, 0);
//    sprintf(buffer, "Time:%02d:%02d:%02d", 
//            time->hours, time->minutes, time->seconds);
//    lcd_print_string(buffer);
//}

