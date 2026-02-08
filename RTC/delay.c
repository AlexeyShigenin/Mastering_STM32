/**
  ******************************************************************************
  * @file			delay.c
  * @brief		Модуль программных задержек
  * @details	Реализует задержки в микросекундах и миллисекундах
	* @author		Алексей Шигенин
  ******************************************************************************
  */

#include "delay.h"
#include "stm32f10x.h"      // Device header

#define SYSCLOCK 72000000U	// Константа системной частоты для организации задержек

/**
  * @brief  Задержка в миллисекундах
  * @param  ms: количество миллисекунд
  * @retval None
  */
void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms; i++) {
        for (uint32_t j = 0; j < 7200; j++) {
            __asm__("nop");
        }
    }
}

/**
  * @brief  Задержка в микросекундах
  * @param  us: количество микросекунд
  * @retval None
  */
void delay_us(uint32_t us) {
    for (uint32_t i = 0; i < us; i++) {
        for (uint32_t j = 0; j < 7; j++) {
            __asm__("nop");
        }
    }
}


////	Закоментированный код - вариант реализации задержек на SysTick
///**
//  * @brief  Задержка в миллисекундах
//  * @param  ms: количество миллисекунд
//  * @retval None
//  */
//void delay_ms(uint32_t ms) {
//	for (uint32_t i = 0; i < ms; i++) {
//		SysTick->LOAD = (SYSCLOCK / 10000) - 1;  // 1 мс (7200-1 так как счетчик доходит до 0, то ставим 7199)
//		SysTick->VAL = 0;
//		SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;										// Включаем SysTick
//		while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);	// Ждем, пока счетчик дойдет до 0
//		SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;									// Отключаем SysTick
//		}
//}

///**
//  * @brief  Задержка в микросекундах
//  * @param  us: количество микросекунд
//  * @retval None
//  */
//void delay_us(uint32_t us) {
//	for (uint32_t i = 0; i < us; i++) {
//		SysTick->LOAD = (SYSCLOCK / 10000000) - 1; // 1 мкс (7-1 так как счетчик доходит до 0, то ставим 6)
//		SysTick->VAL = 0;
//		SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;										// Включаем SysTick
//		while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);	// Ждем, пока счетчик дойдет до 0
//		SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;									// Отключаем SysTick
//	}
//}
