// Программа мигания светодиодом на основе библиотеки CMSIS (STM32F103RB)

#include "stm32f10x.h"                // Device header

int main(void){
	
	int const pause = 4500000;					// Константа, определяющая частоту мигания светодиода
	
	// Включаем тактирование PORTA
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
		
	// Конфигурируем PORTA PIN5 как general purpose output push-pull, max speed 2 MHz 
	// Нужно настроить: MODE5_0 = 0; MODE5_1 = 1; CNF5_0 = 0; CNF5_1 = 0
	GPIOA->CRL |= GPIO_CRL_MODE5_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF5_0;
	
	while(1){														// В бесконечном цикле
		GPIOA->BSRR |= GPIO_BSRR_BS5;			// Включаем светодиод
		for (int i=0; i < pause; i++){}		// Пауза
		GPIOA->BSRR |= GPIO_BSRR_BR5;			// Выключаем светодиод
		for (int i=0; i < pause; i++){}		// Пауза
	}
}
