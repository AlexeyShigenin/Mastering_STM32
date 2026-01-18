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
  * @brief  Инициализация всех GPIO
  * @param  None
  * @retval None
  */
void gpio_init(void) {
    // Включение тактирования GPIOA и GPIOB
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
    
    // Настройка PB2 (светодиод) как выход Push-Pull, 50 MHz
    GPIOB->CRL &= ~(GPIO_CRL_CNF2 | GPIO_CRL_MODE2);
    GPIOB->CRL |= GPIO_CRL_MODE2_0 | GPIO_CRL_MODE2_1;  // 50 MHz
    GPIOB->CRL |= GPIO_CRL_CNF2_0;  // Push-pull
    
    // Выключение светодиода по умолчанию
    GPIOB->ODR &= ~GPIO_ODR_ODR2;
}
