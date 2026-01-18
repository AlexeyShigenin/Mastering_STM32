/**
  ******************************************************************************
  * @file    scheduler.h
  * @brief   Заголовочный файл для модуля планировщика
  ******************************************************************************
  */

#ifndef __SCHEDULER_H
#define __SCHEDULER_H

#include "rtc.h"

/* Состояние внешнего устройства */
#define DEVICE_OFF 0
#define DEVICE_ON  1

/* Прототипы функций */
void scheduler_init(void);
void scheduler_check(RTC_TimeTypeDef* time, RTC_DateTypeDef* date);
void scheduler_set_on_time(RTC_TimeTypeDef* time);
void scheduler_set_off_time(RTC_TimeTypeDef* time);
void scheduler_set_date(RTC_DateTypeDef* date);
uint8_t scheduler_get_device_state(void);

#endif /* __SCHEDULER_H */
