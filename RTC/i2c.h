/**
  ******************************************************************************
  * @file    i2c.h
  * @brief   Заголовочный файл для модуля I2C
  ******************************************************************************
  */

#ifndef __I2C_H
#define __I2C_H

#include <stdint.h>

/* Адрес PCF8574T по умолчанию (A0-A2 = 0) */
#define PCF8574_ADDRESS 0x4E

/* Прототипы функций */
void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_write(uint8_t data);
uint8_t i2c_read_ack(void);
uint8_t i2c_read_nack(void);
void i2c_write_byte(uint8_t addr, uint8_t data);

#endif /* __I2C_H */
