/**
  ******************************************************************************
  * @file    rtc.h
  * @brief   Заголовочный файл для модуля RTC
  ******************************************************************************
  */

#ifndef __RTC_H
#define __RTC_H

#include <stdint.h>

/* Структура для хранения времени */
typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} RTC_TimeTypeDef;

/* Структура для хранения даты */
typedef struct {
    uint8_t day;
    uint8_t month;
    uint8_t year;  // последние две цифры года
    uint8_t weekday; // 1-7: Понедельник-Воскресенье
} RTC_DateTypeDef;

/* Структура для расписания */
typedef struct {
    uint8_t enabled;
    RTC_TimeTypeDef on_time;
    RTC_TimeTypeDef off_time;
    RTC_DateTypeDef date;
} ScheduleTypeDef;

/* Функции модуля */
void rtc_init(void);
void RTC_SetDateTime(RTC_TimeTypeDef* time, RTC_DateTypeDef* date);
void RTC_GetDateTime(RTC_TimeTypeDef* time, RTC_DateTypeDef* date);
uint32_t rtc_convert_to_seconds(RTC_TimeTypeDef* time);
void rtc_convert_from_seconds(uint32_t seconds, RTC_TimeTypeDef* time);

#endif /* __RTC_H */
