/**
  ******************************************************************************
  * @file    scheduler.c
  * @brief   Модуль управления расписанием включения/выключения устройства
  * 
  * Модуль сравнивает текущее время с заданным расписанием
  * и управляет состоянием внешнего устройства через GPIO.
  * Также управляет светодиодом-индикатором состояния.
  ******************************************************************************
  */

#include "scheduler.h"
#include "stm32f10x.h"                  // Device header
#include "gpio.h"

extern ScheduleTypeDef device_schedule;

/* Текущее состояние устройства */
static uint8_t device_state = DEVICE_OFF;

/**
  * @brief  Инициализация планировщика
  * @param  None
  * @retval None
  */
void scheduler_init(void) {
    device_state = DEVICE_OFF;
    
    // Настройка вывода для управления внешним устройством (PB1)
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    
    // PB1 как выход Push-Pull, 50 MHz
    GPIOB->CRL &= ~(GPIO_CRL_CNF1 | GPIO_CRL_MODE1);
    GPIOB->CRL |= GPIO_CRL_MODE1_0 | GPIO_CRL_MODE1_1;  // 50 MHz
    GPIOB->CRL |= GPIO_CRL_CNF1_0;  // Push-pull
    
    // Выключение устройства по умолчанию
    GPIOB->ODR &= ~GPIO_ODR_ODR1;
}

/**
  * @brief  Проверка расписания и управление устройством
  * @param  time: текущее время
  * @param  date: текущая дата
  * @retval None
  */
void scheduler_check(RTC_TimeTypeDef* time, RTC_DateTypeDef* date) {
    // Проверка активности расписания
    if (!device_schedule.enabled) {
        return;
    }
    
    // Проверка совпадения даты
    if (date->day == device_schedule.date.day &&
        date->month == device_schedule.date.month &&
        date->year == device_schedule.date.year) {
        
        // Конвертация времени в секунды для сравнения
        uint32_t current_seconds = time->hours * 3600 + time->minutes * 60 + time->seconds;
        uint32_t on_seconds = device_schedule.on_time.hours * 3600 + 
                              device_schedule.on_time.minutes * 60 + 
                              device_schedule.on_time.seconds;
        uint32_t off_seconds = device_schedule.off_time.hours * 3600 + 
                               device_schedule.off_time.minutes * 60 + 
                               device_schedule.off_time.seconds;
        
        // Проверка времени включения
        if (current_seconds >= on_seconds && current_seconds < off_seconds) {
            if (device_state != DEVICE_ON) {
                device_state = DEVICE_ON;
                GPIOB->ODR |= GPIO_ODR_ODR1;  // Включение устройства
                GPIOB->ODR |= GPIO_ODR_ODR2;  // Включение светодиода
            }
        }
        // Проверка времени выключения
        else if (current_seconds >= off_seconds) {
            if (device_state != DEVICE_OFF) {
                device_state = DEVICE_OFF;
                GPIOB->ODR &= ~GPIO_ODR_ODR1;  // Выключение устройства
                GPIOB->ODR &= ~GPIO_ODR_ODR2;  // Выключение светодиода
            }
        }
    } else {
        // Если дата не совпадает, выключаем устройство
        if (device_state != DEVICE_OFF) {
            device_state = DEVICE_OFF;
            GPIOB->ODR &= ~GPIO_ODR_ODR1;  // Выключение устройства
            GPIOB->ODR &= ~GPIO_ODR_ODR2;  // Выключение светодиода
        }
    }
}

/**
  * @brief  Установка времени включения
  * @param  time: время включения
  * @retval None
  */
void scheduler_set_on_time(RTC_TimeTypeDef* time) {
    device_schedule.on_time.hours = time->hours;
    device_schedule.on_time.minutes = time->minutes;
    device_schedule.on_time.seconds = time->seconds;
    device_schedule.enabled = 1;
}

/**
  * @brief  Установка времени выключения
  * @param  time: время выключения
  * @retval None
  */
void scheduler_set_off_time(RTC_TimeTypeDef* time) {
    device_schedule.off_time.hours = time->hours;
    device_schedule.off_time.minutes = time->minutes;
    device_schedule.off_time.seconds = time->seconds;
}

/**
  * @brief  Установка даты расписания
  * @param  date: дата
  * @retval None
  */
void scheduler_set_date(RTC_DateTypeDef* date) {
    device_schedule.date.day = date->day;
    device_schedule.date.month = date->month;
    device_schedule.date.year = date->year;
}

/**
  * @brief  Получение текущего состояния устройства
  * @param  None
  * @retval Состояние устройства (DEVICE_ON или DEVICE_OFF)
  */
uint8_t scheduler_get_device_state(void) {
    return device_state;
}
