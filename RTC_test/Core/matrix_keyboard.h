/**
  ******************************************************************************
  * @file			matrix_keyboard.c
  * @brief		Заголовочный файл функций опроса матричной клавиатуры
  ******************************************************************************
  */
	
#ifndef MATRIX_KEYBOARD_H_
#define MATRIX_KEYBOARD_H_

#include "stm32f10x.h"      			// Device header

/* Прототипы функций */
void keyboardInit(void);
int scanKeyboard(void);
int getKeyPress(void);
void keyboardProcessKey(int key);


#endif /* MATRIX_KEYBOARD_H_ */
