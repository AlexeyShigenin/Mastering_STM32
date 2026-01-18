/**
	******************************************************************************
	* @file			main.c
	* @brief		Основной файл программы
	* @author		Алексей Шигенин	
	******************************************************************************
	*/

#include "stm32f10x.h"                  // Device header
#include "Core/sysinit.h"
#include "Core/i2c.h"
#include "Core/lcd.h"
#include "Core/delay.h"


int main(void)
{
	sysClockTo72();		// Настройка тактирования на 72 МГц
	i2cInit();				// Инициализация I2C
	
	
	lcd_init();
	
	lcd_set_cursor(0,0);
	lcd_print_string("Alexey");
	delay_ms(1000);
	lcd_set_cursor(1,0);
	lcd_print_string("Shigenin");
	delay_ms(1000);
	lcd_clear();
	delay_ms(1000);
	lcd_set_cursor(0,0);
	lcd_print_string("Shigenin");
	delay_ms(1000);
	lcd_set_cursor(1,0);
	lcd_print_string("Alexey");
}
