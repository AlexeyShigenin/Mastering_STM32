
//--------- Используется модуль USART2 ----------

#include "stm32f10x.h"                  		// Device header

int main(void){
	
	//SystemInit();		// Инициализация по-умолчанию (в данном случае можно не использовать)
	
	// Включение тактирования
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;				// Тактирование порта А
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;				// Тактирование порта B
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;				// Тактирование альтернативных функций
	
	// Настройка вывода на передачу	
	// Вывод PORTA2 - вывод TX, необходимо настроить как:
	// - Alternate Function output, push-pull (CNF0 = 0, CNF1 = 1)
	// - max output speed = 2MHz (MODE0 = 0, MODE1 = 1) В книге было 10MHz
	GPIOA->CRL &= ~GPIO_CRL_CNF2_0;						// Сброс бита CNF0
	GPIOA->CRL |=  GPIO_CRL_CNF2_1;						// Установка бита CNF1
	GPIOA->CRL &= ~GPIO_CRL_MODE2_0;					// Сброс бита MODE0
	GPIOA->CRL |=  GPIO_CRL_MODE2_1;					// Установка бита MODE1
	
	// Настройка вывода на прием
	// Вывод PORTA3 - вывод RX, необходимо настроить как:
	// Input floating (CNF0 = 1, CNF1 = 0, MODE0 = 0, MODE1 = 0)
	GPIOA->CRL |=  GPIO_CRL_CNF3_0;						// Установка бита CNF0
	GPIOA->CRL &= ~GPIO_CRL_CNF3_1;						// Сброс бита CNF1
	GPIOA->CRL &= ~GPIO_CRL_MODE3;						// Сброс битов MODE0, MODE1
	
	// Настройка USART2
	// Для скорости обмена baudrate = 9600 бод и частоты fck = PCLK1(APB1) = 36MHz
	// USARTDIV = fck / 16*baudrate = 36000000 / 16*9600 = 0d234.375
	// DIV_Mantissa = 0d234 = 0xEA (целая часть USARTDIV)
	// DIV_Fraction = 0d0.375 * 16 = 0d6 = 0x6
	// Соответственно, USART_BRR = DIV_MantisaDIV_Fraction = 0xEA6
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;			// Включение тактирования USART2	
	USART2->BRR = 0xEA6;											// Настройка скорости обмена
	USART2->CR1 |= USART_CR1_UE;							// USART Enable - включение USART
	USART2->CR1 |= USART_CR1_TE;							// Transmitter Enable - включение передатчика
	USART2->CR1 |= USART_CR1_RE;							// Receiver Enable - включение приемника
	USART2->CR1 |= USART_CR1_RXNEIE;					// Разрешение прерывания от RXNE (регистр приема не пуст)
	
	// Настройка вывода PORTB2 (светодиод):
	// - General purpose output, push-pull (CNF0 = 0, CNF1 = 0)
	// - max output speed = 2MHz (MODE0 = 0, MODE1 = 1)
	GPIOB->CRL &= ~GPIO_CRL_CNF2;							// Сброс битов CNF0, CNF1
	GPIOB->CRL &= ~GPIO_CRL_MODE2_0;					// Сброс бита MODE0
	GPIOB->CRL |=  GPIO_CRL_MODE2_1;					// Установка бита MODE1
	
	// Включение контроллера прерываний NVIC по вектору прерывания USART1_IRQn
	//NVIC_EnableIRQ(USART2_IRQn);						// Включение с помощью функции CMSIS NVIC_EnableIRQ()
	NVIC->ISER[1] |= (1<<6);									// Можно включить также непосредственно в регистре NVIC_ISER[]
																						// см. RM0056 стр. 120.
																						// USART2_IRQn = 38 (файл stm32f10x.h)
																						// номера прерываний 0-31 - биты 0-31 в регистре NVIC_ISER[0]
																						// номера прерываний 32-63 - биты 0-31 в регистре NVIC_ISER[1]
																						// соответственно, номер прерывания 38 это бит 6 в NVIC_ISER[1]
	
	// Глобальное разрешение прерываний
	__enable_irq();
	
	while(1) {
		static char data=0;											// Переменная для отправки данных на ПК
		for (uint32_t i=0; i<2000000; ++i) {		// Задержка для удобного отслеживания результата программы
		}
		USART2->DR = data++;										// Загрузка данных в регистр DR для передачи
		while ((USART2->SR & USART_SR_TC)==0) {	// Ожидание окончания передачи - ожидание установки флага TC (TransmitComplete)
		}																				// чтобы новые инкрементированные данные (data++) не записались в регистр DR
																						// пока передача не завершилась (чтобы не испортить предыдущие загруженные данные)
		USART2->SR &= ~USART_SR_TC;							// Когда данные переданы, очищаем флаг окончания передачи
																						// для возможности дальнейшей передачи
		//USART2->SR = ~USART_SR_TC;						// В книге флаг очищается таким способом
	}
}

// Обработчик прерывания
void USART2_IRQHandler(void) {
	char input=0;															// Переменная для сохранения принятых байтов
	if ((USART2->SR & USART_SR_RXNE)!=0) {		// Проверяем установку флага RXNE (RXNotEmpty)
		input = USART2->DR;											// Если флаг установлен - считываем данные из регистра данных
																						// при этом, чтение из регистра данных также одновременно сбрасывает флаг RXNE
		if (input == '1') {
			GPIOB->ODR ^= GPIO_ODR_ODR2;					// Если в принятых данных '1' - то меняем состояние светодиода
		}
		else if (input == '2') {
			USART2->DR = 0xAE;										// Если в принятых данных '2' - то в регистр данных загружаем символ с кодом ASCII 0xAE
																						// и данный символ будет передан через UART
		}
	}
}
