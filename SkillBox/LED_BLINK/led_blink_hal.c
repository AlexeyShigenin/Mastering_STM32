// Программа мигания светодиодом на основе библиотеки HAL (STM32F407VGT6)

#include "stm32f4xx.h"                  // Keil::Device:Startup
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "stm32f4xx_hal_conf.h"         // Keil::Device:STM32Cube Framework:Classic

int main(void){
	
	int const pause = 4500000;						// Константа, определяющая частоту мигания светодиода
	
	// Включаем тактирование PORTD
	__HAL_RCC_GPIOD_CLK_ENABLE();
		
	// Конфигурируем PORTD PIN12 
	// Создаем экземпляр структуры GPIO_InitTypeDef
	GPIO_InitTypeDef LED;
	// Заполняем структуру
	LED.Pin = GPIO_PIN_12;
	LED.Mode = GPIO_MODE_OUTPUT_PP;
	LED.Speed = GPIO_SPEED_FREQ_LOW;
	LED.Pull = GPIO_NOPULL;
	// Инициализируем GPIO структурой LED
	HAL_GPIO_Init(GPIOD, &LED);
	
	while(1){																									// В бесконечном цикле
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);		// Включаем светодиод
		for (int i=0; i < pause; i++){}													// Пауза
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);	// Выключаем светодиод
		for (int i=0; i < pause; i++){}													// Пауза
	}
}
