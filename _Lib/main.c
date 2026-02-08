#include "stm32f10x.h"                  // Device header
#include "_Lib/delay/delay.h"

void pinB2init (void){	// Реализция функция инициализации PORTB PIN2, к которому подключен светодиод
		RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // Включаем тактирование поррта B
		
		// Настраиваем PORTB PIN2 на выход - MODE_0 и MODE_1 lдля данного пина нужно установить не равным 0b00 (устанавливаем 0b10 для частоты 2МГц)
		GPIOB->CRL &= ~GPIO_CRL_MODE2_0;	//	Устанавливаем MODE0 для PIN2
		GPIOB->CRL |= GPIO_CRL_MODE2_1;		//	Сбрасываем MODE1 для PIN2
		
		// Конфигурируем PORTB PIN2 в режим Push-Pull (CNF1=0, CNF0=0)
		GPIOB->CRL &= ~GPIO_CRL_CNF2_0;	//CNF0=0
		GPIOB->CRL &= ~GPIO_CRL_CNF2_1;	//CNF1=0
	}

int main(void){
	
	pinB2init();
	DWTDelay_Init();
	SysTickDelay_Init();
	
	uint32_t last_led_toggle = getDWTCountDelay();
	while(1){
		if(delayDWT_nb_us(last_led_toggle,1000000)){
			GPIOB->ODR ^= GPIO_ODR_ODR2;
			last_led_toggle = getDWTCountDelay();
		}	
	}
}
