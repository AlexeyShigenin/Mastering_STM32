// Программа мигания светодиодом на основе библиотеки SPL (STM32F103RB)

#include "stm32f10x.h"                  // Device header
#include "stm32f10x_rcc.h"              // Keil::Device:StdPeriph Drivers:RCC
#include "stm32f10x_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO

int main(void){
	
	int const pause = 4500000;						// Константа, определяющая частоту мигания светодиода
	
	// Включаем тактирование PORTA
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		
	// Конфигурируем PORTA PIN5 как general purpose output push-pull, max speed 2 MHz 
	// Создаем экземпляр структуры GPIO_InitTypeDef
	GPIO_InitTypeDef LED;
	// Заполняем структуру
	LED.GPIO_Pin = GPIO_Pin_5;
	LED.GPIO_Mode = GPIO_Mode_Out_PP;
	LED.GPIO_Speed = GPIO_Speed_2MHz;
	// Инициализируем GPIOA структурой LED
	GPIO_Init(GPIOA, &LED);
	
	while(1){															// В бесконечном цикле
		GPIO_SetBits(GPIOA, GPIO_Pin_5);		// Включаем светодиод
		for (int i=0; i < pause; i++){}			// Пауза
		GPIO_ResetBits(GPIOA, GPIO_Pin_5);	// Выключаем светодиод
		for (int i=0; i < pause; i++){}			// Пауза
	}
}
