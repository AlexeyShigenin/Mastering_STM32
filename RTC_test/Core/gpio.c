/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   Модуль инициализации портов ввода/вывода
  * 
  * Инициализирует все используемые GPIO:
  * - PB2: светодиод-индикатор состояния устройства
  * - PB1: управление внешним устройством
  * - PA0-PA3: кнопки управления
  ******************************************************************************
  */

#include "gpio.h"
#include "stm32f10x.h"                  // Device header

/**
	******************************************************************************
	* @brief	Инициализация всех GPIO
	* @param	None
	* @retval None
	*/
void gpioInit(void) {
	// Включение тактирования GPIOA и GPIOB
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

	// Настройка PB1 (управление внешним устройством) как выход Push-Pull, 2 MHz
	// CNF1_0=0, CNF1_1=0, MODE1_0=0, MODE1_1=1
	GPIOB->CRL &= ~(GPIO_CRL_CNF1 | GPIO_CRL_MODE1);
	GPIOB->CRL |= GPIO_CRL_MODE1_1;
	// Выключение устройства по умолчанию
	GPIOB->BSRR |= GPIO_BSRR_BR1;
	
	// Настройка PB2 (светодиод) как выход Push-Pull, 2 MHz
	// CNF2_0=0, CNF2_1=0, MODE2_0=0, MODE2_1=1
	GPIOB->CRL &= ~(GPIO_CRL_CNF2 | GPIO_CRL_MODE2);
	GPIOB->CRL |= GPIO_CRL_MODE2_1;
	// Выключение светодиода по умолчанию
	GPIOB->BSRR |= GPIO_BSRR_BR2;
}
