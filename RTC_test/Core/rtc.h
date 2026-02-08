/**
  ******************************************************************************
  * @file			rtc.h
  * @brief		Заголовочный файл модуля Функций для работы с RTC
  ******************************************************************************
  */

#ifndef RTC_H
#define RTC_H

#include "stm32f10x.h"

/* Структура для хранения времени и даты */
typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t weekday;
} RTCTimeDate;

/* Структура для расписания */
typedef struct {
    uint8_t enabled;
    RTCTimeDate on_time;
    RTCTimeDate off_time;
} ScheduleTypeDef;

/* Функции модуля */
void rtcInit(void);											// Инициализация RTC
void RTCSetTimeDate(RTCTimeDate *td);
void RTCGetTimeDate(RTCTimeDate *td);
uint8_t RTCIsLeapYear(uint16_t year);
uint32_t RTCConvertToSeconds(RTCTimeDate *td);
void RTCConvertFromSeconds(uint32_t seconds, RTCTimeDate *td);

#endif	/* RTC */
