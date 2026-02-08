
//--------- Используется модуль USART1 ----------

#include "stm32f10x.h"                  		// Device header

static uint8_t input[20];															// Переменная для сохранения принятых байтов
static uint8_t i=0;

int main(void){
	
	
	//SystemInit();		// Инициализация по-умолчанию (в данном случае можно не использовать)
	
	// Включение тактирования
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;				// Тактирование порта А
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;				// Тактирование порта B
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;				// Тактирование альтернативных функций
	
	// Настройка вывода на передачу	
	// Вывод PORTA9 - вывод TX, необходимо настроить как:
	// - Alternate Function output, push-pull (CNF0 = 0, CNF1 = 1)
	// - max output speed = 2MHz (MODE0 = 0, MODE1 = 1) В книге было 10MHz
	GPIOA->CRH &= ~GPIO_CRH_CNF9_0;						// Сброс бита CNF0
	GPIOA->CRH |=  GPIO_CRH_CNF9_1;						// Установка бита CNF1
	GPIOA->CRH &= ~GPIO_CRH_MODE9_0;					// Сброс бита MODE0
	GPIOA->CRH |=  GPIO_CRH_MODE9_1;					// Установка бита MODE1
	
	// Настройка вывода на прием
	// Вывод PORTA10 - вывод RX, необходимо настроить как:
	// Input floating (CNF0 = 1, CNF1 = 0, MODE0 = 0, MODE1 = 0)
	GPIOA->CRH |=  GPIO_CRH_CNF10_0;					// Установка бита CNF0
	GPIOA->CRH &= ~GPIO_CRH_CNF10_1;					// Сброс бита CNF1
	GPIOA->CRH &= ~GPIO_CRH_MODE10;						// Сброс битов MODE0, MODE1
	
	// Настройка USART1
	// Для скорости обмена baudrate = 9600 бод и частоты fck = PCLK2(APB2) = 72MHz
	// USARTDIV = fck / 16*baudrate = 72000000 / 16*9600 = 0d468.75	= 0x1D4
	// DIV_Mantissa = 0x1D4 (целая часть USARTDIV)
	// DIV_Fraction = 0d0.75 * 16 = 0d12 = 0xC
	// Соответственно, USART_BRR = DIV_MantisaDIV_Fraction = 0x1D4C
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;			// Включение тактирования USART1	
	USART1->BRR = 0x1D4C;											// Настройка скорости обмена
	USART1->CR1 |= USART_CR1_UE;							// USART Enable - включение USART
	USART1->CR1 |= USART_CR1_TE;							// Transmitter Enable - включение передатчика
	USART1->CR1 |= USART_CR1_RE;							// Receiver Enable - включение приемника
	USART1->CR1 |= USART_CR1_RXNEIE;					// Разрешение прерывания от RXNE (регистр приема не пуст)
	
	// Настройка вывода PORTB2 (светодиод):
	// - General purpose output, push-pull (CNF0 = 0, CNF1 = 0)
	// - max output speed = 2MHz (MODE0 = 0, MODE1 = 1)
	GPIOB->CRL &= ~GPIO_CRL_CNF2;							// Сброс битов CNF0, CNF1
	GPIOB->CRL &= ~GPIO_CRL_MODE2_0;					// Сброс бита MODE0
	GPIOB->CRL |=  GPIO_CRL_MODE2_1;					// Установка бита MODE1
	
	// Включение контроллера прерываний NVIC по вектору прерывания USART1_IRQn
	//NVIC_EnableIRQ(USART1_IRQn);						// Включение с помощью функции CMSIS NVIC_EnableIRQ()
	NVIC->ISER[1] |= (1<<5);									// Можно включить также непосредственно в регистре NVIC_ISER[]
																						// см. RM0056 стр. 120.
																						// USART1_IRQn = 37 (файл stm32f10x.h)
																						// номера прерываний 0-31 - биты 0-31 в регистре NVIC_ISER[0]
																						// номера прерываний 32-63 - биты 0-31 в регистре NVIC_ISER[1]
																						// соответственно, номер прерывания 37 это бит 5 в NVIC_ISER[1]
	
	// Глобальное разрешение прерываний
	__enable_irq();


// для проверки modbus нужно отправить F7 06 00 03 00 00 6D 5C
		
		USART1->SR &= ~USART_SR_TC;
		
		USART1->DR = 0xF7;
		while ((USART1->SR & USART_SR_TC)==0) {	// Ожидание окончания передачи - ожидание установки флага TC (TransmitComplete)
		}
		USART1->SR &= ~USART_SR_TC;
		
		USART1->DR = 0x06;
		while ((USART1->SR & USART_SR_TC)==0) {	// Ожидание окончания передачи - ожидание установки флага TC (TransmitComplete)
		}
		USART1->SR &= ~USART_SR_TC;
		
		USART1->DR = 0x00;
		while ((USART1->SR & USART_SR_TC)==0) {	// Ожидание окончания передачи - ожидание установки флага TC (TransmitComplete)
		}
		USART1->SR &= ~USART_SR_TC;
		
		USART1->DR = 0x03;
		while ((USART1->SR & USART_SR_TC)==0) {	// Ожидание окончания передачи - ожидание установки флага TC (TransmitComplete)
		}
		USART1->SR &= ~USART_SR_TC;
		
		USART1->DR = 0x00;
		while ((USART1->SR & USART_SR_TC)==0) {	// Ожидание окончания передачи - ожидание установки флага TC (TransmitComplete)
		}
		USART1->SR &= ~USART_SR_TC;
		
		USART1->DR = 0x00;
		while ((USART1->SR & USART_SR_TC)==0) {	// Ожидание окончания передачи - ожидание установки флага TC (TransmitComplete)
		}
		USART1->SR &= ~USART_SR_TC;
		
		USART1->DR = 0x6D;
		while ((USART1->SR & USART_SR_TC)==0) {	// Ожидание окончания передачи - ожидание установки флага TC (TransmitComplete)
		}
		USART1->SR &= ~USART_SR_TC;
		
		USART1->DR = 0x5C;
		while ((USART1->SR & USART_SR_TC)==0) {	// Ожидание окончания передачи - ожидание установки флага TC (TransmitComplete)
		}
		USART1->SR &= ~USART_SR_TC;

	while(1) {
//		uint8_t data;											// Переменная для отправки данных на ПК
		
//		for (uint32_t i=0; i<2000000; ++i) {		// Задержка для удобного отслеживания результата программы
//		}
//		USART1->DR = data++;										// Загрузка данных в регистр DR для передачи
//		while ((USART1->SR & USART_SR_TC)==0) {	// Ожидание окончания передачи - ожидание установки флага TC (TransmitComplete)
//		}																				// чтобы новые инкрементированные данные (data++) не записались в регистр DR
																						// пока передача не завершилась (чтобы не испортить предыдущие загруженные данные)
//		USART1->SR &= ~USART_SR_TC;							// Когда данные переданы, очищаем флаг окончания передачи
																						// для возможности дальнейшей передачи
//		//USART1->SR = ~USART_SR_TC;						// В книге флаг очищается таким способом
	}
}

// Обработчик прерывания
void USART1_IRQHandler(void) {


	if ((USART1->SR & USART_SR_RXNE)!=0) {		// Проверяем установку флага RXNE (RXNotEmpty)
		input[i] = USART1->DR;											// Если флаг установлен - считываем данные из регистра данных
		i++;
		}
	}
