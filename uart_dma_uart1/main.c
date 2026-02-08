#include "stm32f10x.h"                  		// Device header
#include "string.h"													// Для использования функции memcpy

#define RXSIZE 20														// Размер буфера приема куда будем первично складывать байты, пришедшие по UART
uint8_t ReceivedBuffer[20];									// Первичный буфер - массив на 20 элементов
uint8_t MainBuffer[50];											// Основной буффер - массив на 50 элементов
uint8_t Index = 0;													// Переменная, которая показывает количество ходов по буферу

void UART1_Config (void){	
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
	// Для скорости обмена baudrate = 115200 бод и частоты fck = PCLK2(APB2) = 72MHz
	// USARTDIV = fck / 16*baudrate = 72000000 / 16*9600 = 0d39.0625	= 0x27
	// DIV_Mantissa = 0x27 (целая часть USARTDIV)
	// DIV_Fraction = 0d0.0625 * 16 = 0d1 = 0x1
	// Соответственно, USART_BRR = DIV_MantisaDIV_Fraction = 0x271
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;			// Включение тактирования USART1	
	USART1->BRR = 0x271;											// Настройка скорости обмена
	USART1->CR1 |= USART_CR1_UE;							// USART Enable - включение USART
	USART1->CR1 |= USART_CR1_TE;							// Transmitter Enable - включение передатчика
	USART1->CR1 |= USART_CR1_RE;							// Receiver Enable - включение приемника
	USART1->CR3 |= USART_CR3_DMAR;						// Разрешение приема через DMA
	USART1->CR3 |= USART_CR3_DMAT;						// Разрешение передачи через DMA

	// Настройка вывода PORTB2 (светодиод):
	// - General purpose output, push-pull (CNF0 = 0, CNF1 = 0)
	// - max output speed = 2MHz (MODE0 = 0, MODE1 = 1)
	GPIOB->CRL &= ~GPIO_CRL_CNF2;							// Сброс битов CNF0, CNF1
	GPIOB->CRL &= ~GPIO_CRL_MODE2_0;					// Сброс бита MODE0
	GPIOB->CRL |=  GPIO_CRL_MODE2_1;					// Установка бита MODE1
}

void DMA_Init (void){
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;					// Включение тактирования DMA1
	DMA1_Channel5->CCR |= DMA_CCR5_TCIE;			// Разрешение прерывания от события завершения обмена (TransferCompleteInterruptEnable)
	DMA1_Channel5->CCR |= DMA_CCR5_HTIE;			// Разрешение прерывания от события завершения половины обмена (HalfTransferInterruptEnable)
	DMA1_Channel5->CCR |= DMA_CCR5_TEIE;			// Разрешение прерывания от события ошибки обмена (TransferErrorInterruptEnable)
	DMA1_Channel5->CCR &= ~DMA_CCR5_DIR;			// Направление обмена - чтение из периферии (из UART в память)
	DMA1_Channel5->CCR |= DMA_CCR5_CIRC;			// Включение циклического режима
	DMA1_Channel5->CCR |= DMA_CCR5_MINC;			// Режим инкремента адреса памяти для сохранения пришедших данных
	DMA1_Channel5->CCR &= ~DMA_CCR5_PSIZE;		// Размер элемента данных в перифериии - 8 бит (PSIZE = 00)
	DMA1_Channel5->CCR &= ~DMA_CCR5_MSIZE;		// Размер элемента данных в памяти - 8 бит (MSIZE = 00)
	DMA1_Channel5->CCR &= ~DMA_CCR5_PL;				// Уровень приоритета канала низкий (PL = 00)
}

void DMA_Config (uint32_t source, uint32_t destination, uint16_t datasize){
																						// source - адрес источника, destination - адрес назначения, datasize - размер данных
	DMA1_Channel5->CNDTR = datasize;					// устанавливаем размер данных в регистре DMA_CNDTR - можно записывать только когда канал выключен
																						// при включенном канале - возможно только чтение. Регистр декрементируется после каждой DMA транзакции
	DMA1_Channel5->CPAR = source;							// Устанавливаем адрес периферии
	DMA1_Channel5->CMAR = destination;				// Устанавливаем адрес памяти
	DMA1_Channel5->CCR |= DMA_CCR5_EN;				// Включаем канал
}

void DMA1_Channel5_IRQHandler (void){
	if (DMA1->ISR & DMA_ISR_HTIF5){						// Если установлен флаг завершения половины обмена
		
		// Побайтное копирование данных из буфера ReceivedBuffer[0] в MainBuffer[Index] размером 10 байт (RXSIZE/2)
		memcpy(&MainBuffer[Index], &ReceivedBuffer[0], RXSIZE/2);
		
		DMA1->IFCR |= DMA_IFCR_CHTIF5;					// Очистка флага прерывания от заполнения половины буфера (записью в 1 в ссответствуюющий бит регистра DMA_IFCR)
		
		// Увеличиваем значение индекса (номеров) копируемых байтов потока и при превышении им pначения 49 - обнуляем
		Index += RXSIZE/2;
		if (Index>49){
			Index = 0;														// Если индекс превысит размер основого буфера - сбрасываем ешо
		}
		GPIOB->BSRR |= GPIO_BSRR_BS2;						// Включаем светодиод
	}
	
	if (DMA1->ISR & DMA_ISR_TCIF5){						// Если установлен флаг завершения полного обмена
		
		// Побайтное копирование данных из буфера ReceivedBuffer[RXSIZE/2] в MainBuffer[Index] размером 10 байт (RXSIZE/2)
		memcpy(&MainBuffer[Index], &ReceivedBuffer[RXSIZE/2], RXSIZE/2);
	
		DMA1->IFCR |= DMA_IFCR_CTCIF5;						// Очистка флага прерывания от полного заполнения буфера (записью в 1 в ссответствуюющий бит регистра DMA_IFCR)
		
		// Увеличиваем значение индекса (номеров) копируемых байтов потока и при превышении им pначения 49 - обнуляем
		Index += RXSIZE/2;
		if (Index>49){
			Index = 0;														// Если индекс превысит размер основого буфера - сбрасываем ешо
		}
		GPIOB->BSRR |= GPIO_BSRR_BR2;						// Выключаем светодиод
	}
}

int main(){
	UART1_Config();
	DMA_Init();
	NVIC_SetPriority(DMA1_Channel5_IRQn,0);		// Установка приоритета обработчика прерывания в 0
	NVIC_EnableIRQ(DMA1_Channel5_IRQn);				// Включение прерывания
	// Вызов функции пересылки данных из регистра DR в буфер ReceivedBuffer размером RXSIZE
	DMA_Config((uint32_t) &USART1->DR, (uint32_t) ReceivedBuffer, RXSIZE);
	while (1){
	}
}
