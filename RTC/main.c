/**
  ******************************************************************************
  * @file    main.c
  * @brief   Основной файл программы часов с управлением по расписанию
  * 
  * Программа реализует:
  * 1. Часы реального времени с использованием RTC (LSE 32.768 kHz)
  * 2. Установку времени через UART и клавиатуру (4 кнопки)
  * 3. Управление внешним устройством по расписанию
  * 4. Вывод времени на LCD 1602 (I2C) и UART
  * 5. Индикацию состояния устройства светодиодом на PB2
  * 
  * Периферия:
  * - RTC: LSE (32.768 kHz)
  * - I2C1: ремаппинг на PB8 (SCL), PB9 (SDA)
  * - USART1: ремаппинг на PB6 (TX), PB7 (RX)
  * - GPIO: PB2 (светодиод), PA0-PA3 (кнопки)
  ******************************************************************************
  */

#include "stm32f10x.h"                  // Device header
#include "rtc.h"
#include "uart.h"
#include "i2c.h"
#include "lcd.h"
#include "keyboard.h"
#include "scheduler.h"
#include "gpio.h"
#include "delay.h"
#include "sysinit.h"
#include <string.h>

/* Глобальные переменные */
volatile uint8_t system_mode = 0;  // 0: нормальный режим, 1: установка времени, 2: установка расписания
volatile uint8_t menu_position = 0; // Позиция в меню

RTC_TimeTypeDef current_time;
RTC_DateTypeDef current_date;

/**
  * @brief  Задержка в миллисекундах (альтернативная реализация)
  * @param  ms: количество миллисекунд
  * @retval None
  */
void delay_ms_simple(uint32_t ms) {
    for (uint32_t i = 0; i < ms; i++) {
        for (uint32_t j = 0; j < 7200; j++) {
            __NOP();
        }
    }
}

/**
  * @brief  Обработчик SysTick (1 мс)
  * @param  None
  * @retval None
  */
void SysTick_Handler(void) {
    static uint32_t tick_counter = 0;
    tick_counter++;
    
    // Обработка клавиатуры каждые 10 мс
    if (tick_counter % 10 == 0) {
        keyboard_scan();
    }
    
       // Обновление дисплея каждые 500 мс
    if (tick_counter % 500 == 0) {
        if (system_mode == 0) {
            RTC_GetDateTime(&current_time, &current_date);
            lcd_update_time(&current_time, &current_date);
            
            uart_send_time(&current_time, &current_date);
            
        }
        
        // Проверка расписания
        scheduler_check(&current_time, &current_date);
    }
    
    if (tick_counter >= 1000) tick_counter = 0;
}

/**
  * @brief  Обработчик прерывания USART1
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void) {
    // Проверка флага приема
    if (USART1->SR & USART_SR_RXNE) {
        char received_char = USART1->DR;
        
        // Передача обратно (эхо)
        while (!(USART1->SR & USART_SR_TXE));
        USART1->DR = received_char;
        
        // Обработка полученного символа
        uart_process_char(received_char);
    }
}

/**
  * @brief  Главная функция программы
  * @param  None
  * @retval int
  */
int main(void) {
    // Настройка тактирования
    sysClockTo72();
    
    // Настройка SysTick (1 мс)
    SysTick_Config(72000);
    
    // Инициализация периферии
    gpio_init();
    rtc_init();
    i2cInit();
    lcd_init();
    keyboard_init();
    scheduler_init();
		uart_init();
    
    // Включение прерываний
    __enable_irq();
    
    // Установка времени по умолчанию
    current_time.hours = 12;
    current_time.minutes = 0;
    current_time.seconds = 0;
    
    current_date.day = 1;
    current_date.month = 1;
    current_date.year = 24;
    current_date.weekday = 1;
    
    // Установка начального времени
    RTC_SetDateTime(&current_time, &current_date);
    
    // Получение текущего времени
    RTC_GetDateTime(&current_time, &current_date);
    
    // Вывод начального сообщения
    lcd_clear();
    lcd_print_string("STM32 Clock System");
    lcd_set_cursor(1, 0);
    lcd_print_string("Initialized...");
    delay_ms(2000);
    
    // Вывод приветствия в UART
    uart_send_string("\r\nSTM32 Clock System Ready\r\n");
    uart_send_string("Type 'help' for commands\r\n");
    
    // Основной цикл
    while (1) {
        // Обработка команд UART (если есть готовые команды)
        uart_process_command();
        
        // Обработка нажатий кнопок
        uint8_t key = keyboard_get_key();
        if (key != 0) {
            keyboard_process_key(key);
        }
        
        // Задержка для снижения нагрузки на ЦП
        delay_ms_simple(10);
    }
}
