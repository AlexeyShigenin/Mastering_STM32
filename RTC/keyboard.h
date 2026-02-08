/**
  ******************************************************************************
  * @file    keyboard.h
  * @brief   Заголовочный файл для модуля клавиатуры
  ******************************************************************************
  */

#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include <stdint.h>


/* Прототипы функций */
void keyboard_init(void);
void keyboard_scan(void);
uint8_t keyboard_get_key(void);
void keyboard_process_key(uint8_t key);

#endif /* __KEYBOARD_H */
