/**
	******************************************************************************
	* @file			main.c
	* @brief		Основной файл программы
	* @author		Алексей Шигенин	
	******************************************************************************
	*/

#include "stm32f10x.h"                  // Device header



#include "Core/sysinit.h"
#include "Core/lcd.h"
// Данные файлы уже включены в lcd.h
//#include "Core/rtc.h"
//#include "Core/i2c.h"
//#include "Core/delay.h"
//#include <stdio.h>

extern RTCTimeDate currentTime;


int main(void) {
    
	sysClockTo72();		// Настройка тактирования на 72 МГц
	DWTDelay_Init();
	i2cInit();				// Инициализация I2C
	lcdInit();
	rtcInit();		// Инициализация RTC
	lcdSetCursor(0,0);

uint32_t lastLCDUpdate = getDWTCountDelay();
//uint32_t last_sensor_read = getDWTCountDelay();
	while (1) {
	// Задача 1: Обновляем данные на LCD дисплее каждые 5000 мС (НЕ БЛОКИРУЕТ!)
		if (delayDWT_nb_ms(lastLCDUpdate, 500)) {
			RTCGetTimeDate(&currentTime);
			lcdUpdateTime(&currentTime);
			lastLCDUpdate = getDWTCountDelay();
	}
//		// Задача 2: Опрашиваем датчик каждые 100 мС (НЕ БЛОКИРУЕТ!)
//	if (delayDWT_nb_ms(last_sensor_read, 100)) {
//		read_sensor();
//		last_sensor_read = getDWTCountDelay();
//	}
// CPU не блокируется, может выполнять полезную работу
// или спать для экономии энергии
	__WFI();
}
			

}
