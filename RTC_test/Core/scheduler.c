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
#include "stm32f10x.h"                  		// Device header
#include "gpio.h"

extern ScheduleTypeDef deviceSchedule;			// структура определена в rtc.c
static uint8_t schedulerState = 0;						// Текущее состояние планировщика
static uint8_t deviceState = 0;							// Текущее состояние устройства

/**
  * @brief  Проверка расписания и управление устройством
  * @param  None
  * @retval None
  */
void schedulerCheck(void) {
	if ((deviceSchedule.secondsCurrent >= deviceSchedule.secondsOn) && (deviceSchedule.secondsCurrent < deviceSchedule.secondsOff)) {
		deviceOn();
	} else if (deviceSchedule.secondsCurrent < deviceSchedule.secondsOn) {
		deviceOffandSchedulerStatusOn();
	} else {
		deviceOff();
	}

}
void deviceOn (void) {
	if (deviceState != DEVICE_ON) {
		GPIOB->ODR |= GPIO_ODR_ODR1;  // Включение устройства
		GPIOB->ODR |= GPIO_ODR_ODR2;  // Включение светодиода
		deviceState = DEVICE_ON;			// Меняем статус устройства
		schedulerState = 1;						// Меняем статус планировщика
	}
}

void deviceOff (void) {
	if (deviceState != DEVICE_OFF) {
		GPIOB->ODR &= ~GPIO_ODR_ODR1;		// Выключение устройства
		GPIOB->ODR &= ~GPIO_ODR_ODR2;  // Выключение светодиода
		deviceState = DEVICE_OFF;				// Меняем статус устройства
		schedulerState = 0;						// Меняем статус планировщика
	}
}

void deviceOffandSchedulerStatusOn (void) {
	if (deviceState != DEVICE_ON) {
		GPIOB->ODR &= ~GPIO_ODR_ODR1;		// Выключение устройства
		GPIOB->ODR &= ~GPIO_ODR_ODR2;  // Выключение светодиода
		deviceState = DEVICE_OFF;				// Меняем статус устройства
		schedulerState = 1;						// Меняем статус планировщика
	}
}

/**
  * @brief  Установка времени включения
  * @param  time: время включения
  * @retval None
  */
void schedulerSetOnTime(RTCTimeDate* timeDate) {
	deviceSchedule.onTime.hours = timeDate->hours;
	deviceSchedule.onTime.minutes = timeDate->minutes;
	deviceSchedule.onTime.seconds = timeDate->seconds;
	deviceSchedule.onTime.day = timeDate->day;
	deviceSchedule.onTime.month = timeDate->month;
	deviceSchedule.onTime.year = timeDate->year;
	deviceSchedule.secondsOn = RTCConvertToSeconds(timeDate);
	
	BKP->DR3 = deviceSchedule.secondsOn & 0xFFFF;
	BKP->DR4 = (deviceSchedule.secondsOn >> 16) & 0xFFFF;
}

/**
  * @brief  Установка времени выключения
  * @param  time: время выключения
  * @retval None
  */
void schedulerSetOffTime(RTCTimeDate* timeDate) {
	deviceSchedule.offTime.hours = timeDate->hours;
	deviceSchedule.offTime.minutes = timeDate->minutes;
	deviceSchedule.offTime.seconds = timeDate->seconds;
	deviceSchedule.offTime.day = timeDate->day;
	deviceSchedule.offTime.month = timeDate->month;
	deviceSchedule.offTime.year = timeDate->year;
	deviceSchedule.secondsOff = RTCConvertToSeconds(timeDate);
	
	BKP->DR5 = deviceSchedule.secondsOff & 0xFFFF;
	BKP->DR6 = (deviceSchedule.secondsOff >> 16) & 0xFFFF;
}

/**
  * @brief  Получение текущего состояния устройства
  * @param  None
  * @retval Состояние устройства (DEVICE_ON или DEVICE_OFF)
  */
uint8_t getDeviceState(void) {
    return deviceState;
}

/**
  * @brief  Получение текущего состояния планировщика
  * @param  None
  * @retval Состояние устройства (DEVICE_ON или DEVICE_OFF)
  */
uint8_t getSchedulerState(void) {
    return schedulerState;
}
