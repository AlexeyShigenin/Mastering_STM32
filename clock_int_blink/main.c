#include "stm32f10x.h"                  // Device header
 
#define LED_PB2_ON()	GPIOB->BSRR |= GPIO_BSRR_BS2;	// Макрос включения светодиода
#define LED_PB2_OFF()	GPIOB->BSRR |= GPIO_BSRR_BR2;	// Макрос выключения светодиода

void setSysClockTo72(void); // Прототип функции настройки тактирования на частоту 72 МГц
void pinB2init (void);				// Прототип функция инициализации PORTB PIN2, к которому подключен светодиод
void pinA0init (void);				// Прототип функция инициализации PORTB PIN2, к которому подключен светодиод
void interruptEXTI_PA0_init (void);	// Прототип функции инициализации прерываний

int pause = 4500000;
 
int main(void){
	setSysClockTo72();				// Вызываем функцию настройки тактирования
	pinB2init();							// Вызываем функцию инициализации PORTB PIN2 (светодиод)
	pinA0init();							// Вызываем функцию инициализации PORTA PIN0 (кнопка)
	interruptEXTI_PA0_init();	// Вызываем функцию настройки прерываний
	
	while(1){
		LED_PB2_OFF();
		for (int i=0; i<pause; i++){
		}
		LED_PB2_ON()
		for (int i=0; i<pause; i++){
		}
	}
}
 
 void setSysClockTo72 (void) {	// Реализация функции настройки тактирования на частоту 72 МГц
	 
		// ---------- Настройка тактирования ----------

		/* Порядок настройки:
		1. Запустить генератор HSE (HighSpeedExternal)
		2. Настроить количество циклов ожидания FLASH (т.к. FLASH может работать максимум на частоте 24 МГц,
																									а FLASH работает на шине AHB, то при частоте шины AHB выше 24М Гц нужно
																									для работы FLASH пропускать несколько циклов (чем выше частота, тем больше циклов пропускать)
		3. Настроить делители шин
		4. Настроить PLL
		5. Запустить PLL
		6. Переключиться на работу от PLL */
	 
	 // 1. Запускаем HSE генератор - в регистре RCC_CR (ResetClockControl_ControlRegister)
		RCC->CR |= RCC_CR_HSEON;	/* Включаем внешнее тактирование:
															устанавливаем бит RCC_CR_HSEON (HighSpeedExternalON) в регистре RCC_CR */

		while (READ_BIT (RCC->CR, RCC_CR_HSERDY) == RESET) {		/* Ожидаем стабилизацию тактирования от HSE:
																														ждем установку флага RCC_CR_HSERDY (HighSpeedExternalReady) в регистре RCC_CR */
		}
		
		// 2. Настройка Flash памяти - регистр FLASH_ACR ((Flash_AccesControlRegister)
		/* Выключаем - запускаем буфер предварительной выборки */
		FLASH->ACR &= ~FLASH_ACR_PRFTBE; /* Отключаем буфер предварительной выборки:
																			сбрасываем бит FLASH_ACR_PRFTBE (PReFeTchBufferEnable) в регистре FLASH_ACR */
		FLASH->ACR |= FLASH_ACR_PRFTBE;	/* Запускаем буфер предварительной выборки:
																			устанавливаем бит FLASH_ACR_PRFTBE (PReFeTchBufferEnable) в регистре FLASH_ACR */

		/* Настраиваем битовое поле управления задержкой (биты LATENCY (0-2) в регистре FLASH_ACR) */
		FLASH->ACR &= ~FLASH_ACR_LATENCY;		/* Сбрасываем предыдущую настройку LATENCY */
		FLASH->ACR |= FLASH_ACR_LATENCY_2;	/* устанавливаем задержку для частоты 72 МГц (LATENCY_2 соответствует 010 */
		
		// 3. Настраиваем делители шин - настраиваем регистр RCC_CFGR (ResetClockControl_ConFiGurationRegister)
		/* Настраиваем AHB Prescaler - AHB предделитель системной частоты SYSCLK для шины AHB (частота HCLK) */
		RCC->CFGR &= ~RCC_CFGR_HPRE;			/* Сбрасываем предыдцущие настройки HPRE (биты 4-7) */
		RCC->CFGR |= RCC_CFGR_HPRE_DIV1;	/* Устанавливаем режим деления частоты SYSCLK на 1, т.е. без деления,
																			соответственно получается, что частота HCLK = SYSCLK */
		/* Коэффициент деления для шины APB2 (APB2 Preskaler) */
		RCC->CFGR &= ~RCC_CFGR_PPRE2;			/* Сбрасываем предыдцущие настройки PPRE2 (биты 11-13) */
		RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;	/* Устанавливаем режим деления частоты HCLK на 1, т.е. без деления,
																			соответственно получается, что частота PCLK2 = HCLK */
		/* Коэффициент деления для шины APB1 (APB1 Prescaler) */
		RCC->CFGR &= ~RCC_CFGR_PPRE1;			/* Сбрасываем предыдцущие настройки PPRE1 (биты 8-10) */
		RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;	/* Устанавливаем режим деления частоты HCLK на 2,
																			соответственно получается, что частота PCLK1 = HCLK/2 */
		// 4. Настраиваем PLL мультиплексор */
		RCC->CFGR &= (uint32_t) ((uint32_t) ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));	/* Сбрасываем биты предыдущей настройки PLL
																																																		(биты можно менять только при выключенном PLL):
																																																		RCC_CFGR_PLLSRC (бит 16) - источник входа PLL
																																																		RCC_CFGR_PLLXTPRE (бит 17) — Делитель с HSE генератора перед подачей на PLL
																																																		RCC_CFGR_PLLMULL (биты 18-21) - коэффициент умножения PLL */
		RCC->CFGR |= (uint32_t) (RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMULL9);															/* Конфигурируем PLL:
																																																		RCC_CFGR_PLLSRC (бит 16) - устанавливаем бит - источник входа PLL - HSE
																																																		RCC_CFGR_PLLMULL9 (биты 18-21: 0111) - коэффициент умножения PLL=9 */
		// 5. Запускаем PLL
		RCC->CR |= RCC_CR_PLLON;	/* Включаем PLL: устанавливаем бит PLLON (бит 24) в регистре RCC_CR */
		while (READ_BIT (RCC->CR, RCC_CR_PLLRDY)!=(RCC_CR_PLLRDY)) {	/* После включения PLL ожидаем его готовности (ждем установки флага RCC_CR_PLLRDY) */
		}
		// 6. Переключаемся на PLL - на System Clock Mux выбираем вход PLLCLK
		RCC->CFGR &= ~RCC_CFGR_SW;	/* Сбрасываем предыдущие настройки RCC_CFRG_SW (SWitch system clock - переключатель источника тактирования) - биты 0 и 1 в RCC_CFGR */
		RCC->CFGR |= RCC_CFGR_SW_PLL; /* Выбираем в качестве источника PLL:
																		00: HSI выбран в качестве источника системного тактирования
																		01: HSE выбран в качестве источника системного тактирования
																		10: PLL выбран в качестве источника системного тактирования*/
		while (READ_BIT (RCC->CFGR, RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) { /* Ожидаем пока PLL не будет использоваться в качестве источника тактирования:
																																	RCC_CFGR_SWS - флаги статуса переключателя (SWitchStatus) источника тактирования
																																	ожидаем, пока они не установятся аппаратно в состояние RCC_CFGR_SWS_PLL */
		}
	}
 
	void pinB2init (void){	// Реализция функция инициализации PORTB PIN2, к которому подключен светодиод
		RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // Включаем тактирование поррта B
		
		// Настраиваем PORTB PIN2 на выход - MODE_0 и MODE_1 lдля данного пина нужно установить не равным 0b00 (устанавливаем 0b10 для частоты 2МГц)
		GPIOB->CRL &= ~GPIO_CRL_MODE2_0;	//	Устанавливаем MODE0 для PIN2
		GPIOB->CRL |= GPIO_CRL_MODE2_1;		//	Сбрасываем MODE1 для PIN2
		
		// Конфигурируем PORTB PIN2 в режим Push-Pull (CNF1=0, CNF0=0)
		GPIOB->CRL &= ~GPIO_CRL_CNF2_0;	//CNF0=0
		GPIOB->CRL &= ~GPIO_CRL_CNF2_1;	//CNF1=0
	}
	
	void pinA0init (void){	// Реализция функция инициализации PORTA PIN0, к которому подключена кнопка
		RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; // Включаем тактирование поррта A
		
		// Настраиваем PORTA PIN0 на вход - MODE_0 и MODE_1 lдля данного пина нужно сбросить (00)
		GPIOA->CRL &= ~GPIO_CRL_MODE0_0;	//	Сбрасываем MODE0 для PIN0
		GPIOA->CRL &= ~GPIO_CRL_MODE0_1;	//	Сбрасываем MODE1 для PIN0
		
		// Конфигурируем PORTA PIN0 на вход с подтяжкой к питанию или земле
		GPIOA->CRL &= ~GPIO_CRL_CNF0_0;	//CNF0=0
		GPIOA->CRL |= GPIO_CRL_CNF0_1;	//CNF1=1
		
		GPIOA->BSRR |= GPIO_BSRR_BR0; // Подтяжка пина к земле для зациты от помех
	}
	
	 void interruptEXTI_PA0_init (void){	// Реализация функции инициализации прерываний
		 EXTI->PR |= EXTI_PR_PR0;	// Сбрасываем флаг прерывания перед включением самого прерывания (PendingRegister)
		 EXTI->IMR |= EXTI_IMR_MR0;	// Размаскируем (1) прерывания 0-го канала EXTI (InterruptMaskRegister)
		 AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI0_PA; // Нулевой канал EXT1 подключаем к порту PA0 (EXTernalInterruptConfigurationRegister)
		 RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;	// Включаем тактирование для AFIO (AlternateFunctionIO) PortA
		 EXTI->FTSR |=EXTI_FTSR_TR0;	// Включаем (1) прерывание по спаду импульса (FallingTriggerSelectionRegister)
		 
		 //NVIC_EnableIRQ (EXTI0_IRQn); // Разрешаем прерывания в контроллере прерываний на мультиплексоре EXTI0_IRQn
		 NVIC->ISER[0] |= (1<<6); /* Тоже самое, но настройка не через функцию а через регистр NVIC_ISER (PM0056 стр. 120)
															Описания регистров NVIC нет в RM0008, т.к. они относятся не к микроконтроллеру, а к ядру CORTEX-M3 */
		 
		 NVIC_SetPriority (EXTI0_IRQn, 0);	//Устанавливаем приоритет прерывания
	 }
	 
	 // Обработчика прерываний. Имя берем из файла startup_stm32f10x_md.s (Startup)
	 void EXTI0_IRQHandler (void){
		 if (READ_BIT (EXTI->PR, EXTI_PR_PR0)){	// Проверяем флаг прерывания (PendingRegister)
			 EXTI->PR |= EXTI_PR_PR0;	// Если флаг установленБ сбрасываем флаг прерывания записью "1"
			 if(pause>300000){	// Меняем задержку мигания светодиода
				 pause /= 2;
			 }
			 else pause = 10000000;
		 }
	 }
	
