/**
	******************************************************************************
	* @file			sysint.c
	* @brief		Инициализация системы
	* @author		Алексей Шигенин
	******************************************************************************
	*/

#include "sysinit.h"
#include "stm32f10x.h"

/*******************************************************************************
	* @brief		Настройка тактированая на частоту 72 МГц
	* @param		None
	* @retval		None
	******************************************************************************
	*/
void sysClockTo72(void){
	// 1.	Включение HSE - в регистр RCC_CR (ResetClockControl_ControlRegister)
	//		устанавливаем бит RCC_CR_HSEON (HighSpeedExternalON)
	RCC->CR |= RCC_CR_HSEON;
	// 		Ожидаем готовность HSE - ждем установку флага
	//		RCC_CR_HSERDY (HighSpeedExternalReady) в регистре RCC_CR
	while(!(RCC->CR & RCC_CR_HSERDY));

	// 2.	Настройка Flash памяти - регистр FLASH_ACR ((Flash_AccesControlRegister)
	// 		Выключаем - включаем буфер предварительной выборки
	// 		Отключаем буфер предварительной выборки - сбрасываем бит
	//		FLASH_ACR_PRFTBE (PReFeTchBufferEnable) в регистре FLASH_ACR
	FLASH->ACR &= ~FLASH_ACR_PRFTBE;
	// 		Запускаем буфер предварительной выборки - устанавливаем бит
	//		FLASH_ACR_PRFTBE (PReFeTchBufferEnable) в регистре FLASH_ACR
	FLASH->ACR |= FLASH_ACR_PRFTBE;
	// 		Настраиваем битовое поле управления задержкой
	//		(биты LATENCY (0-2) в регистре FLASH_ACR)
	//		Сбрасываем предыдущую настройку LATENCY
	FLASH->ACR &= ~FLASH_ACR_LATENCY;
	//		устанавливаем задержку для частоты 72 МГц (LATENCY_2 соответствует 010)
	FLASH->ACR |= FLASH_ACR_LATENCY_2;

	// 3.	Настраиваем делители шин - настраиваем регистр
	//		RCC_CFGR (ResetClockControl_ConFiGurationRegister)
	// 		Настраиваем AHB Prescaler
	//		AHB предделитель системной частоты SYSCLK для шины AHB (частота HCLK)
	// 		Сбрасываем предыдцущие настройки HPRE (биты 4-7)
	RCC->CFGR &= ~RCC_CFGR_HPRE;				
	//		Устанавливаем режим деления частоты SYSCLK на 1, т.е. без деления,
	//		соответственно получается, что частота HCLK = SYSCLK
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
	//		Коэффициент деления для шины APB1 (APB1 Prescaler)
	//		Сбрасываем предыдцущие настройки PPRE1 (биты 8-10)
	RCC->CFGR &= ~RCC_CFGR_PPRE1;
	// 		Устанавливаем режим деления частоты HCLK на 2, соответственно
	//		получается, что частота PCLK1 = HCLK/2
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
	// 		Коэффициент деления для шины APB2 (APB2 Preskaler)
	// 		Сбрасываем предыдцущие настройки PPRE2 (биты 11-13)
	RCC->CFGR &= ~RCC_CFGR_PPRE2;
	// 		Устанавливаем режим деления частоты HCLK на 1, т.е. без деления,
	//		соответственно получается, что частота PCLK2 = HCLK
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;

	// 4.	Настраиваем PLL мультиплексор
	// 		Сбрасываем биты предыдущей настройки PLL
	//		(биты можно менять только при выключенном PLL):
	RCC->CFGR &= ~((RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
	//		RCC_CFGR_PLLSRC (бит 16) - источник входа PLL
	RCC->CFGR &= ~RCC_CFGR_PLLSRC;
	//		RCC_CFGR_PLLXTPRE — Делитель с HSE генератора перед подачей на PLL
	RCC->CFGR &= ~RCC_CFGR_PLLXTPRE;
	//		RCC_CFGR_PLLMULL - коэффициент умножения PLL
	RCC->CFGR &= ~RCC_CFGR_PLLMULL;
	//		Конфигурируем PLL:
	//		RCC_CFGR_PLLSRC (бит 16) - источник входа PLL - HSE
	RCC->CFGR |= RCC_CFGR_PLLSRC_HSE;
	//		RCC_CFGR_PLLMULL9 (биты 18-21: 0111) - коэффициент умножения PLL=9
	RCC->CFGR |= RCC_CFGR_PLLMULL9;			
	
	// 5. Запускаем PLL
	//		Включаем PLL: устанавливаем бит PLLON (бит 24) в регистре RCC_CR
	RCC->CR |= RCC_CR_PLLON;
	//		После включения PLL ожидаем его готовности
	//		(ждем установки флага RCC_CR_PLLRDY)
	while(!(RCC->CR & RCC_CR_PLLRDY));
	
	// 6.	Переключаемся на PLL - на System Clock Mux выбираем вход PLLCLK
	// 		Сбрасываем предыдущие настройки RCC_CFRG_SW (SWitch system clock)
	RCC->CFGR &= ~RCC_CFGR_SW;
	// 		Выбираем в качестве источника PLL:
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	//		Ожидаем пока PLL не будет использоваться в качестве источника
	//		тактирования (RCC_CFGR_SWS, RCC_CFGR_SWS_PLL - флаги статуса
	//		переключателя (SWitchStatus) источника тактирования)
	// 		ожидаем, пока RCC_CFGR_SWS не установятся аппаратно в RCC_CFGR_SWS_PLL
	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}
