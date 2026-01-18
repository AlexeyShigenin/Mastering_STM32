/**
  ******************************************************************************
  * @file    keyboard.h
  * @brief   Заголовочный файл для модуля клавиатуры
  ******************************************************************************
  */

#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include <stdint.h>

/* Коды кнопок */
#define KEY_NONE     0
#define KEY_UP       1
#define KEY_DOWN     2
#define KEY_LEFT     3
#define KEY_RIGHT    4
#define KEY_ENTER    5
#define KEY_ESC      6

/* Прототипы функций */
void keyboard_init(void);
void keyboard_scan(void);
uint8_t keyboard_get_key(void);
void keyboard_process_key(uint8_t key);

#endif /* __KEYBOARD_H */
