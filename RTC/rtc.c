/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   Модуль работы с часами реального времени (RTC)
  * 
  * Модуль использует LSE (Low Speed External) кварц 32.768 kHz
  * для точного измерения времени. Реализует установку и чтение
  * времени/даты через регистры RTC.
  ******************************************************************************
  */

#include "rtc.h"
#include "stm32f10x.h"                  // Device header

/* Глобальная структура для хранения расписания */
ScheduleTypeDef device_schedule = {0};

/**
  * @brief  Инициализация RTC
  * @param  None
  * @retval None
  */
void rtc_init(void) {
    // Включение тактирования PWR и BKP
    RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;
    
    // Разрешение доступа к области BKP
    PWR->CR |= PWR_CR_DBP;
    
    // Сброс флага RTC для инициализации (если нужно)
    // В CMSIS для F1 нет BKP_CSR_TPE, используем прямой доступ
    // BKP->CSR |= (1 << 0); // TPE bit - необязательно для базовой работы
    
    // Включение LSE (32.768 kHz)
    RCC->BDCR |= RCC_BDCR_LSEON;
    while (!(RCC->BDCR & RCC_BDCR_LSERDY));
    
    // Выбор LSE как источника RTC
    RCC->BDCR &= ~RCC_BDCR_RTCSEL;
    RCC->BDCR |= RCC_BDCR_RTCSEL_LSE;
    
    // Включение тактирования RTC
    RCC->BDCR |= RCC_BDCR_RTCEN;
    
    // Ожидание синхронизации RTC
    RTC->CRL &= ~RTC_CRL_RSF;
    while (!(RTC->CRL & RTC_CRL_RSF));
    
    // Разблокировка регистров RTC
    RTC->CRL |= RTC_CRL_CNF;
    
    // Настройка предделителя для 32.768 kHz
    // 32768/(32767+1) = 1 Hz (1 секунда)
    RTC->PRLH = 0x00;
    RTC->PRLL = 0x7FFF;
    
    // Блокировка регистров RTC
    RTC->CRL &= ~RTC_CRL_CNF;
}

/**
  * @brief  Установка времени и даты
  * @param  time: указатель на структуру времени
  * @param  date: указатель на структуру даты
  * @retval None
  */
void RTC_SetDateTime(RTC_TimeTypeDef* time, RTC_DateTypeDef* date) {
    uint32_t counter;
    
    // Конвертация времени в секунды
    counter = time->hours * 3600 + time->minutes * 60 + time->seconds;
    
    // Разблокировка регистров RTC
    RTC->CRL |= RTC_CRL_CNF;
    
    // Запись значения в счетчик RTC
    RTC->CNTL = counter & 0xFFFF;
    RTC->CNTH = (counter >> 16) & 0xFFFF;
    
    // Блокировка регистров RTC
    RTC->CRL &= ~RTC_CRL_CNF;
    
    // Ожидание завершения операции
    while (!(RTC->CRL & RTC_CRL_RTOFF));
    
    // Сохранение даты в резервных регистрах BKP
    PWR->CR |= PWR_CR_DBP;  // Разрешение доступа к BKP
    
    // Дата сохраняется в BKP_DR1 (32 бита)
    // Формат: 0xYYYYMMDDWW (год, месяц, день, день недели)
    uint32_t date_data = (date->year << 24) | (date->month << 16) | 
                         (date->day << 8) | date->weekday;
    BKP->DR1 = date_data;
    
    // Запись времени в BKP_DR2 (32 бита)
    // Формат: 0xHHMMSS00
    uint32_t time_data = (time->hours << 24) | (time->minutes << 16) | 
                         (time->seconds << 8);
    BKP->DR2 = time_data;
    
    // Дополнительно сохраняем расписание в BKP_DR3
    if (device_schedule.enabled) {
        uint32_t schedule_data = (device_schedule.on_time.hours << 24) | 
                                 (device_schedule.on_time.minutes << 16) | 
                                 (device_schedule.on_time.seconds << 8) |
                                 device_schedule.date.day;
        BKP->DR3 = schedule_data;
        
        // Время выключения и дата в BKP_DR4
        uint32_t schedule_data2 = (device_schedule.off_time.hours << 24) | 
                                  (device_schedule.off_time.minutes << 16) | 
                                  (device_schedule.off_time.seconds << 8) |
                                  device_schedule.date.month;
        BKP->DR4 = schedule_data2;
        
        // Год и флаг активности в BKP_DR5
        uint32_t schedule_data3 = (device_schedule.date.year << 8) | 
                                  device_schedule.enabled;
        BKP->DR5 = schedule_data3;
    }
    
    PWR->CR &= ~PWR_CR_DBP; // Запрет доступа к BKP
}

/**
  * @brief  Чтение текущего времени и даты
  * @param  time: указатель на структуру времени
  * @param  date: указатель на структуру даты
  * @retval None
  */
void RTC_GetDateTime(RTC_TimeTypeDef* time, RTC_DateTypeDef* date) {
    uint32_t counter, date_data;
    
    // Ожидание флага завершения операции
    while (!(RTC->CRL & RTC_CRL_RTOFF));
    
    // Чтение счетчика RTC
    counter = RTC->CNTL;
    counter |= (RTC->CNTH << 16);
    
    // Конвертация секунд в часы, минуты, секунды
    time->hours = counter / 3600;
    time->minutes = (counter % 3600) / 60;
    time->seconds = counter % 60;
    
    // Чтение даты из BKP_DR1
    PWR->CR |= PWR_CR_DBP;
    date_data = BKP->DR1;
    PWR->CR &= ~PWR_CR_DBP;
    
    // Извлечение даты
    if (date_data != 0) {
        date->year = (date_data >> 24) & 0xFF;
        date->month = (date_data >> 16) & 0xFF;
        date->day = (date_data >> 8) & 0xFF;
        date->weekday = date_data & 0xFF;
    } else {
        // Значения по умолчанию
        date->year = 24;
        date->month = 1;
        date->day = 1;
        date->weekday = 1;
    }
    
    // Восстановление расписания из резервных регистров
    if (BKP->DR5 != 0) {
        device_schedule.enabled = BKP->DR5 & 0xFF;
        device_schedule.date.year = (BKP->DR5 >> 8) & 0xFF;
        
        uint32_t sched1 = BKP->DR3;
        uint32_t sched2 = BKP->DR4;
        
        device_schedule.on_time.hours = (sched1 >> 24) & 0xFF;
        device_schedule.on_time.minutes = (sched1 >> 16) & 0xFF;
        device_schedule.on_time.seconds = (sched1 >> 8) & 0xFF;
        device_schedule.date.day = sched1 & 0xFF;
        
        device_schedule.off_time.hours = (sched2 >> 24) & 0xFF;
        device_schedule.off_time.minutes = (sched2 >> 16) & 0xFF;
        device_schedule.off_time.seconds = (sched2 >> 8) & 0xFF;
        device_schedule.date.month = sched2 & 0xFF;
    }
}

/**
  * @brief  Конвертация времени в секунды
  * @param  time: указатель на структуру времени
  * @retval Количество секунд
  */
uint32_t rtc_convert_to_seconds(RTC_TimeTypeDef* time) {
    return time->hours * 3600 + time->minutes * 60 + time->seconds;
}

/**
  * @brief  Конвертация секунд во время
  * @param  seconds: количество секунд
  * @param  time: указатель на структуру времени
  * @retval None
  */
void rtc_convert_from_seconds(uint32_t seconds, RTC_TimeTypeDef* time) {
    time->hours = seconds / 3600;
    time->minutes = (seconds % 3600) / 60;
    time->seconds = seconds % 60;
}
