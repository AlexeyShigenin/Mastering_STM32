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
void schedulerCheck(void);
void schedulerSetOnTime(RTCTimeDate*);
void schedulerSetOffTime(RTCTimeDate*);
uint8_t getDeviceState(void);		// Получение информации о состоянии устройства
uint8_t getSchedulerState(void); // Получение информации о вкл. или выкл. расписании
void deviceOn (void);
void deviceOff (void);
void deviceOffandSchedulerStatusOn (void);

#endif /* __SCHEDULER_H */
