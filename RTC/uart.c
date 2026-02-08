/**
  ******************************************************************************
  * @file    uart.c
  * @brief   Модуль работы с UART (USART1)
  * 
  * USART1 настроен на ремаппинг выводов:
  * PB6 - TX, PB7 - RX
  * Скорость: 9600 бод
  * Используется для:
  * 1. Вывода текущего времени
  * 2. Установки времени/даты
  * 3. Настройки расписания
  * 4. Отображения состояния системы
  ******************************************************************************
  */

#include "uart.h"
#include "stm32f10x.h"                  // Device header
#include <string.h>
#include <stdlib.h>

/* Глобальные переменные для буфера команд */
static char uart_buffer[UART_BUFFER_SIZE];
static uint8_t uart_buffer_index = 0;
static uint8_t command_ready = 0;

/* Внешние переменные */
extern volatile uint8_t system_mode;
extern volatile uint8_t menu_position;
extern RTC_TimeTypeDef current_time;
extern RTC_DateTypeDef current_date;
extern ScheduleTypeDef device_schedule;

/**
  * @brief  Инициализация USART1
  * @param  None
  * @retval None
  */
void uart_init(void) {
    // Включение тактирования USART1 и GPIOB
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
    
    // Ремаппинг USART1 на PB6(TX), PB7(RX)
    AFIO->MAPR |= AFIO_MAPR_USART1_REMAP;
    
    // Настройка PB6 как альтернативная функция Push-Pull (TX) - 50 MHz
    GPIOB->CRL &= ~(GPIO_CRL_CNF6 | GPIO_CRL_MODE6);
    GPIOB->CRL |= GPIO_CRL_CNF6_1;  // Alternate function output Push-pull
    GPIOB->CRL |= GPIO_CRL_MODE6_0 | GPIO_CRL_MODE6_1;  // 50 MHz
    
    // Настройка PB7 как вход с подтяжкой (RX)
    GPIOB->CRL &= ~(GPIO_CRL_CNF7 | GPIO_CRL_MODE7);
    GPIOB->CRL |= GPIO_CRL_CNF7_0;  // Input with pull-up/pull-down
    GPIOB->ODR |= GPIO_ODR_ODR7;    // Pull-up
    
    // Настройка скорости 9600 бод (при 72 MHz)
    // USARTDIV = 72000000 / (16 * 9600) = 468.75
    // DIV_Mantissa = 468 = 0x1D4
    // DIV_Fraction = 0.75 * 16 = 12 = 0xC
    USART1->BRR = (468 << 4) | 0xC; // 0x1D4C
    
    // Включение передатчика и приемника
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;
    
    // Включение прерывания по приему
    USART1->CR1 |= USART_CR1_RXNEIE;
    
    // Включение USART1
    USART1->CR1 |= USART_CR1_UE;
    
    // Включение прерывания USART1 в NVIC
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, 0);
}

/**
  * @brief  Отправка одного символа
  * @param  c: символ для отправки
  * @retval None
  */
void uart_send_char(char c) {
    // Ожидание готовности передатчика
    while (!(USART1->SR & USART_SR_TXE));
    
    // Отправка символа
    USART1->DR = c;
}

/**
  * @brief  Отправка строки
  * @param  str: указатель на строку
  * @retval None
  */
void uart_send_string(const char* str) {
    while (*str) {
        uart_send_char(*str++);
    }
}

/**
  * @brief  Отправка времени и даты
  * @param  time: указатель на структуру времени
  * @param  date: указатель на структуру даты
  * @retval None
  */
void uart_send_time(RTC_TimeTypeDef* time, RTC_DateTypeDef* date) {
    char buffer[64];
    
    // Форматирование строки времени
    sprintf(buffer, "Date: %02d/%02d/20%02d Time: %02d:%02d:%02d\r\n",
            date->day, date->month, date->year,
            time->hours, time->minutes, time->seconds);
	
		uart_send_string(buffer);
	
}

/**
  * @brief  Вывод справки по командам
  * @param  None
  * @retval None
  */
void uart_help(void) {
    uart_send_string("\r\n=== Clock System Commands ===\r\n");
    uart_send_string("set time HH:MM:SS - Set current time\r\n");
    uart_send_string("set date DD:MM:YY - Set current date\r\n");
    uart_send_string("schedule on HH:MM:SS - Set device ON time\r\n");
    uart_send_string("schedule off HH:MM:SS - Set device OFF time\r\n");
    uart_send_string("schedule date DD:MM:YY - Set schedule date\r\n");
    uart_send_string("enable schedule - Enable scheduling\r\n");
    uart_send_string("disable schedule - Disable scheduling\r\n");
    uart_send_string("status - Show current status\r\n");
    uart_send_string("help - Show this help\r\n");
    uart_send_string("=============================\r\n");
}

/**
  * @brief  Обработка символа из UART
  * @param  c: полученный символ
  * @retval None
  */
void uart_process_char(char c) {
    // Обработка конца строки
    if (c == '\r' || c == '\n') {
        if (uart_buffer_index > 0) {
            uart_buffer[uart_buffer_index] = '\0'; // Завершаем строку
            command_ready = 1; // Устанавливаем флаг готовности команды
        }
    }
    // Обработка backspace
    else if (c == 0x08 || c == 0x7F) { // Backspace или Delete
        if (uart_buffer_index > 0) {
            uart_buffer_index--;
            uart_send_string("\b \b"); // Удаление символа в терминале
        }
    }
    // Добавление символа в буфер
    else if (uart_buffer_index < UART_BUFFER_SIZE - 1) {
        uart_buffer[uart_buffer_index++] = c;
    }
}

/**
  * @brief  Обработка команды из буфера
  * @param  None
  * @retval None
  */
void uart_process_command(void) {
    char* token;
    char* saveptr;
    
    if (!command_ready) {
        return;
    }
    
    command_ready = 0;
    
    uart_send_string("\r\n");
    
    // Удаление возможных пробелов в начале
    char* cmd = uart_buffer;
    while (*cmd == ' ') cmd++;
    
    // Парсинг команды "set time"
    if (strstr(cmd, "set time") == cmd) {
        token = strtok_r(cmd + 9, ":", &saveptr);
        if (token) {
            current_time.hours = atoi(token);
            token = strtok_r(NULL, ":", &saveptr);
            if (token) {
                current_time.minutes = atoi(token);
                token = strtok_r(NULL, " ", &saveptr);
                if (token) {
                    current_time.seconds = atoi(token);
                    RTC_SetDateTime(&current_time, &current_date);
                    uart_send_string("Time set successfully\r\n");
                }
            }
        }
    }
    // Парсинг команды "set date"
    else if (strstr(cmd, "set date") == cmd) {
        token = strtok_r(cmd + 9, ":", &saveptr);
        if (token) {
            current_date.day = atoi(token);
            token = strtok_r(NULL, ":", &saveptr);
            if (token) {
                current_date.month = atoi(token);
                token = strtok_r(NULL, " ", &saveptr);
                if (token) {
                    current_date.year = atoi(token);
                    RTC_SetDateTime(&current_time, &current_date);
                    uart_send_string("Date set successfully\r\n");
                }
            }
        }
    }
    // Парсинг команды "schedule on"
    else if (strstr(cmd, "schedule on") == cmd) {
        token = strtok_r(cmd + 12, ":", &saveptr);
        if (token) {
            device_schedule.on_time.hours = atoi(token);
            token = strtok_r(NULL, ":", &saveptr);
            if (token) {
                device_schedule.on_time.minutes = atoi(token);
                token = strtok_r(NULL, " ", &saveptr);
                if (token) {
                    device_schedule.on_time.seconds = atoi(token);
                    uart_send_string("ON time set successfully\r\n");
                }
            }
        }
    }
    // Парсинг команды "schedule off"
    else if (strstr(cmd, "schedule off") == cmd) {
        token = strtok_r(cmd + 13, ":", &saveptr);
        if (token) {
            device_schedule.off_time.hours = atoi(token);
            token = strtok_r(NULL, ":", &saveptr);
            if (token) {
                device_schedule.off_time.minutes = atoi(token);
                token = strtok_r(NULL, " ", &saveptr);
                if (token) {
                    device_schedule.off_time.seconds = atoi(token);
                    uart_send_string("OFF time set successfully\r\n");
                }
            }
        }
    }
    // Парсинг команды "schedule date"
    else if (strstr(cmd, "schedule date") == cmd) {
        token = strtok_r(cmd + 14, ":", &saveptr);
        if (token) {
            device_schedule.date.day = atoi(token);
            token = strtok_r(NULL, ":", &saveptr);
            if (token) {
                device_schedule.date.month = atoi(token);
                token = strtok_r(NULL, " ", &saveptr);
                if (token) {
                    device_schedule.date.year = atoi(token);
                    uart_send_string("Schedule date set successfully\r\n");
                }
            }
        }
    }
    // Команда "enable schedule"
    else if (strstr(cmd, "enable schedule") == cmd) {
        device_schedule.enabled = 1;
        uart_send_string("Schedule enabled\r\n");
    }
    // Команда "disable schedule"
    else if (strstr(cmd, "disable schedule") == cmd) {
        device_schedule.enabled = 0;
        uart_send_string("Schedule disabled\r\n");
    }
    // Команда status
    else if (strstr(cmd, "status") == cmd) {
        uart_send_time(&current_time, &current_date);
        
        uart_send_string("Schedule: ");
        uart_send_string(device_schedule.enabled ? "Enabled\r\n" : "Disabled\r\n");
        
        if (device_schedule.enabled) {
            char buffer[64];
            sprintf(buffer, "ON: %02d:%02d:%02d, OFF: %02d:%02d:%02d\r\n",
                    device_schedule.on_time.hours,
                    device_schedule.on_time.minutes,
                    device_schedule.on_time.seconds,
                    device_schedule.off_time.hours,
                    device_schedule.off_time.minutes,
                    device_schedule.off_time.seconds);
            uart_send_string(buffer);
            
            sprintf(buffer, "Date: %02d/%02d/20%02d\r\n",
                    device_schedule.date.day,
                    device_schedule.date.month,
                    device_schedule.date.year);
            uart_send_string(buffer);
        }
    }
    // Команда help
    else if (strstr(cmd, "help") == cmd) {
        uart_help();
    }
    // Неизвестная команда
    else if (strlen(cmd) > 0) {
        uart_send_string("Unknown command: ");
        uart_send_string(cmd);
        uart_send_string("\r\nType 'help' for available commands\r\n");
    }
    
    // Очистка буфера
    uart_buffer_index = 0;
    memset(uart_buffer, 0, UART_BUFFER_SIZE);
    
    // Вывод приглашения
    uart_send_string("> ");
}
