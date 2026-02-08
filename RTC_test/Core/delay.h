/**
  ******************************************************************************
  * @file			delay.c
  * @brief		Заголовочный файл функций задержек
  ******************************************************************************
  */

#ifndef DELAY_H_
#define DELAY_H_

#include "stm32f10x.h"      			// Device header

/**
	******************************************************************************
	*			Включение неообходимого типа задержек (раскомментировать)
	******************************************************************************
	*/
#define DWT_DELAY_ENABLE
//#define SYSTICK_DELAY_ENABLE
/**
	******************************************************************************
	*			Задержки на основе модуля DWT (Data Watchpoint and Trace)
	******************************************************************************
	*/
#ifdef DWT_DELAY_ENABLE

void DWTDelay_Init(void);							// Инициализация DWT

uint32_t getDWTCountDelay(void);			// Получение текущего значения счетчика

void delayDWT_us(uint32_t);						// Блокирующая задержка в микросекундах

void delayDWT_ms(uint32_t);						// Блокирующая задержка в миллисекундах

uint8_t delayDWT_nb_us(uint32_t, uint32_t);	// НЕблокирующая задержка
																						// в микросекундах
uint8_t delayDWT_nb_ms(uint32_t, uint32_t);	// НЕблокирующая задержка
																						// в миллисекундах

/*
// Пример использования НЕблокирующей задержки в основном цикле:
DWTDelay_Init();

uint32_t last_led_toggle = getDWTCountDelay();
uint32_t last_sensor_read = getDWTCountDelay();
	*
while (1) {
	// Задача 1: Мигаем светодиодом каждые 500 мС (НЕ БЛОКИРУЕТ!)
	if (delayDWT_nb_ms(last_led_toggle, 500)) {
		GPIO_Toggle(GPIOC, GPIO_Pin_13);
		last_led_toggle = getDWTCountDelay();
	}

// Задача 2: Опрашиваем датчик каждые 100 мС (НЕ БЛОКИРУЕТ!)
	if (delayDWT_nb_ms(last_sensor_read, 100)) {
		read_sensor();
		last_sensor_read = getDWTCountDelay();
	}

// Задача 3: Проверяем кнопку постоянно
	check_button();

// Задача 4: Обрабатываем UART
	process_uart();

// CPU не блокируется, может выполнять полезную работу
// или спать для экономии энергии
	__WFI();
}
*/

#endif /* DWT_DELAY_ENABLE */

/**
	******************************************************************************
	*						Задержки в миллисекундах на основе модуля SysTick
	******************************************************************************
	*/
#ifdef SYSTICK_DELAY_ENABLE

static volatile uint32_t countDelay;

void SysTickDelay_Init(void);					// Инициализация SysTick

void SysTick_Handler(void);						// Обработчик прерываний SysTick

uint32_t getSysTickCountDelay(void);	// Получение текущего значения счетчика

void delaySysTick_ms(uint32_t);				// Блокирующая задержка в миллисекундах

uint8_t delaySysTick_nb_ms(uint32_t, uint32_t);	// НЕблокирующая задержка
																								// в миллисекундах на SysTick
/*
// Пример использования НЕблокирующей задержки в основном цикле:
SysTickDelay_Init();

uint32_t last_led_toggle = getSysTickCountDelay();
uint32_t last_sensor_read = getSysTickCountDelay();
	*
while (1) {
	// Задача 1: Мигаем светодиодом каждые 500 мС (НЕ БЛОКИРУЕТ!)
	if (delaySysTick_nb_ms(last_led_toggle, 500)) {
		GPIO_Toggle(GPIOC, GPIO_Pin_13);
		last_led_toggle = getSysTickCountDelay();
	}

// Задача 2: Опрашиваем датчик каждые 100 мС (НЕ БЛОКИРУЕТ!)
	if (delaySysTick_nb_ms(last_sensor_read, 100)) {
		read_sensor();
		last_sensor_read = getSysTickCountDelay();
	}

// Задача 3: Проверяем кнопку постоянно
	check_button();

// Задача 4: Обрабатываем UART
	process_uart();

// CPU не блокируется, может выполнять полезную работу
// или спать для экономии энергии
	__WFI();
}
*/

#endif /* SYSTICK_DELAY_ENABLE */


/**
	******************************************************************************
	*													Простые задержки на циклах
	******************************************************************************
	*/
void delaySimple_us(uint32_t);	// Блокирующая задержка в микросекундах


#endif /* DELAY_H_ */
