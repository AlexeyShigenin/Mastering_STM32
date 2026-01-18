/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   Модуль работы с I2C (I2C1)
  * 
  * I2C1 настроен на ремаппинг выводов:
  * PB8 - SCL, PB9 - SDA
  * Частота: 100 kHz (стандартный режим)
  * Используется для связи с LCD через PCF8574T
  ******************************************************************************
  */

#include "i2c.h"
#include "stm32f10x.h"                  // Device header
#include "delay.h"

/**
  * @brief  Инициализация I2C1
  * @param  None
  * @retval None
  */
void i2c_init(void) {
    // Включение тактирования I2C1 и GPIOB
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
    
    // Ремаппинг I2C1 на PB8(SCL), PB9(SDA)
    AFIO->MAPR |= AFIO_MAPR_I2C1_REMAP;
    
    // Настройка PB8 и PB9 как альтернативная функция с открытым стоком
    GPIOB->CRH &= ~(GPIO_CRH_CNF8 | GPIO_CRH_MODE8 |
                   GPIO_CRH_CNF9 | GPIO_CRH_MODE9);
    GPIOB->CRH |= GPIO_CRH_CNF8_0 | GPIO_CRH_MODE8_0 |  // SCL: AF Open-drain, 10MHz
                  GPIO_CRH_CNF9_0 | GPIO_CRH_MODE9_0;   // SDA: AF Open-drain, 10MHz
    
    // Сброс I2C1
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;
    
    // Настройка частоты (при 36 MHz APB1)
    // CCR = APB1_FREQ / (2 * I2C_FREQ)
    // Для 100 kHz: CCR = 36,000,000 / (2 * 100,000) = 180
    I2C1->CR2 = 36;  // APB1 частота в MHz
    I2C1->CCR = 180; // Настройка делителя
    
    // Максимальное время нарастания (для стандартного режима)
    // TRISE = APB1_FREQ_MHz + 1 = 36 + 1 = 37
    I2C1->TRISE = 37;
    
    // Включение I2C
    I2C1->CR1 |= I2C_CR1_PE;
    
    // Задержка для стабилизации
    delay_ms(10);
}

/**
  * @brief  Генерация условия START
  * @param  None
  * @retval None
  */
void i2c_start(void) {
    // Генерация START
    I2C1->CR1 |= I2C_CR1_START;
    
    // Ожидание флага SB
    while (!(I2C1->SR1 & I2C_SR1_SB));
}

/**
  * @brief  Генерация условия STOP
  * @param  None
  * @retval None
  */
void i2c_stop(void) {
    // Генерация STOP
    I2C1->CR1 |= I2C_CR1_STOP;
    
    // Ожидание завершения
    while (I2C1->CR1 & I2C_CR1_STOP);
}

/**
  * @brief  Запись байта данных
  * @param  data: байт для передачи
  * @retval None
  */
void i2c_write(uint8_t data) {
    // Запись данных в DR
    I2C1->DR = data;
    
    // Ожидание флага TXE
    while (!(I2C1->SR1 & I2C_SR1_TXE));
}

/**
  * @brief  Чтение байта с отправкой ACK
  * @param  None
  * @retval Прочитанный байт
  */
uint8_t i2c_read_ack(void) {
    // Включение ACK
    I2C1->CR1 |= I2C_CR1_ACK;
    
    // Ожидание флага RXNE
    while (!(I2C1->SR1 & I2C_SR1_RXNE));
    
    return I2C1->DR;
}

/**
  * @brief  Чтение байта без ACK
  * @param  None
  * @retval Прочитанный байт
  */
uint8_t i2c_read_nack(void) {
    // Выключение ACK
    I2C1->CR1 &= ~I2C_CR1_ACK;
    
    // Ожидание флага RXNE
    while (!(I2C1->SR1 & I2C_SR1_RXNE));
    
    return I2C1->DR;
}

/**
  * @brief  Запись байта по указанному адресу
  * @param  addr: адрес устройства (7-битный)
  * @param  data: данные для записи
  * @retval None
  */

void i2c_write_byte(uint8_t addr, uint8_t data) {
    i2c_start();
    
    // Отправка адреса устройства в режиме записи
    i2c_write(addr << 1);
    
    // Ожидание флага ADDR
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    (void)I2C1->SR2; // Чтение SR2 для сброса флага
    
    // Отправка данных
    i2c_write(data);
    
    // Ожидание завершения передачи
    while (!(I2C1->SR1 & I2C_SR1_BTF));
    
    i2c_stop();
    
    // Добавляем небольшую задержку для стабильности
    for (volatile uint32_t i = 0; i < 100; i++);
}
