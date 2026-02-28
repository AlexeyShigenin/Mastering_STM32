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
#include "Core/gpio.h"
#include "Core/scheduler.h"
#include "Core/matrix_keyboard.h"
// Данные файлы уже включены в lcd.h
//#include "Core/rtc.h"
//#include "Core/i2c.h"
//#include "Core/delay.h"
//#include <stdio.h>

extern RTCTimeDate currentTime;		//Глобальная структура для хранения времени
extern uint8_t systemMode;
int keyPress =-1;



int main(void) {
    
	sysClockTo72();			// Настройка тактирования на 72 МГц
	DWTDelay_Init();		// Инициализация DWT
	gpioInit();					// Инициализация GPIO
	i2cInit();					// Инициализация I2C
	lcdInit();					// Инициализация LCD
	rtcInit();					// Инициализация RTC
	keyboardInit();			// Инициализация клавиатуры
	


//uint32_t lastLCDUpdate = getDWTCountDelay();
//uint32_t lastKeyboardUpdate = getDWTCountDelay();

//uint32_t last_sensor_read = getDWTCountDelay();
	while (1) {
//	// Задача 1: Обновляем текущее время на LCD дисплее каждые 100 мС
//		if (delayDWT_nb_ms(lastLCDUpdate, 500)) {
//			if (systemMode == 0){
//				RTCGetTimeDate(&currentTime);	// Получение текущего времени
//				lcdUpdateTime(&currentTime);	// Обновление времени на дисплее
//				lastLCDUpdate = getDWTCountDelay();
//			}
//		}
//		// Задача 2: Опрашиваем клавиатуру каждые 1 мС
//		if (delayDWT_nb_ms(lastKeyboardUpdate, 1)) {
//			keyPress = scanKeyboard();
//			lastKeyboardUpdate = getDWTCountDelay();
//		}
	
		// Обработка нажатий кнопок
		keyPress = getKeyPress();
		if (keyPress != -1) {
			keyboardProcessKey(keyPress);
		}
		//delayDWT_ms(5); // если изменить задержку, то нужно изменить и константы длинного/короткого нажатий
	}
}
