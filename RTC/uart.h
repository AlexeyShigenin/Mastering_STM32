/**
  ******************************************************************************
  * @file    uart.h
  * @brief   Заголовочный файл для модуля UART
  ******************************************************************************
  */

#ifndef __UART_H
#define __UART_H

#include <stdint.h>
#include "rtc.h"

/* Максимальная длина команды */
#define UART_BUFFER_SIZE 64

/* Прототипы функций */
void uart_init(void);
void uart_send_char(char c);
void uart_send_string(const char* str);
void uart_send_time(RTC_TimeTypeDef* time, RTC_DateTypeDef* date);
void uart_process_char(char c);
void uart_process_command(void);
void uart_help(void);

#endif /* __UART_H */
