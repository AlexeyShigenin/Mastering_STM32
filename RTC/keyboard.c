/**
  ******************************************************************************
  * @file    keyboard.c
  * @brief   Модуль обработки клавиатуры (4 кнопки)
  * 
  * Кнопки подключены к порту A:
  * PA0 - KEY_UP
  * PA1 - KEY_DOWN
  * PA2 - KEY_ENTER
  * PA3 - KEY_ESC
  * Используется активный низкий уровень (подтяжка к VCC)
  * Обработка дребезга реализована программно
  ******************************************************************************
  */

#include "keyboard.h"
#include "stm32f10x.h"                  // Device header
#include "lcd.h"
#include "rtc.h"
#include "uart.h"

/* Внешние переменные */
extern volatile uint8_t system_mode;
extern volatile uint8_t menu_position;
extern RTC_TimeTypeDef current_time;
extern RTC_DateTypeDef current_date;
extern ScheduleTypeDef device_schedule;

/* Коды кнопок (изменены для 4 кнопок) */
#define KEY_UP     1
#define KEY_DOWN   2
#define KEY_ENTER  3
#define KEY_ESC    4

/* Переменные для обработки клавиатуры */
static uint8_t last_key_state = 0;
static uint32_t debounce_counter = 0;
static uint8_t current_key = 0;
static uint8_t key_pressed = 0;

/**
  * @brief  Инициализация портов для клавиатуры
  * @param  None
  * @retval None
  */
void keyboard_init(void) {
    // Включение тактирования GPIOA
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    
    // Настройка PA0-PA3 как входы с подтяжкой к VCC
    GPIOA->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0 |
                   GPIO_CRL_CNF1 | GPIO_CRL_MODE1 |
                   GPIO_CRL_CNF2 | GPIO_CRL_MODE2 |
                   GPIO_CRL_CNF3 | GPIO_CRL_MODE3);
    GPIOA->CRL |= GPIO_CRL_CNF0_1 | GPIO_CRL_CNF1_1 |
                  GPIO_CRL_CNF2_1 | GPIO_CRL_CNF3_1;  // Input with pull-up/pull-down
    GPIOA->ODR |= GPIO_ODR_ODR0 | GPIO_ODR_ODR1 |
                  GPIO_ODR_ODR2 | GPIO_ODR_ODR3;      // Pull-up
}

/**
  * @brief  Сканирование состояния кнопок
  * @param  None
  * @retval None
  */
void keyboard_scan(void) {
    uint8_t current_state = 0;
    
    // Чтение состояния всех кнопок (инвертируем, так как активный низкий уровень)
    if (!(GPIOA->IDR & GPIO_IDR_IDR0)) current_state |= (1 << 0); // KEY_UP
    if (!(GPIOA->IDR & GPIO_IDR_IDR1)) current_state |= (1 << 1); // KEY_DOWN
    if (!(GPIOA->IDR & GPIO_IDR_IDR2)) current_state |= (1 << 2); // KEY_ENTER
    if (!(GPIOA->IDR & GPIO_IDR_IDR3)) current_state |= (1 << 3); // KEY_ESC
    
    // Обработка дребезга
    if (current_state != last_key_state) {
        debounce_counter = 0;
        last_key_state = current_state;
    } else if (debounce_counter < 20) {  // 20 мс для дребезга
        debounce_counter++;
        if (debounce_counter == 20) {
            // Стабильное состояние
            if (current_state != 0 && !key_pressed) {
                key_pressed = 1;
                // Определение какой именно ключ нажат
                if (current_state & (1 << 0)) current_key = KEY_UP;
                else if (current_state & (1 << 1)) current_key = KEY_DOWN;
                else if (current_state & (1 << 2)) current_key = KEY_ENTER;
                else if (current_state & (1 << 3)) current_key = KEY_ESC;
            }
        }
    }
    
    // Сброс флага нажатия при отпускании кнопки
    if (current_state == 0 && key_pressed) {
        key_pressed = 0;
    }
}

/**
  * @brief  Получение нажатой клавиши
  * @param  None
  * @retval Код клавиши (0 если нет нажатия)
  */
uint8_t keyboard_get_key(void) {
    uint8_t key = current_key;
    current_key = 0;  // Сброс после чтения
    return key;
}

/**
  * @brief  Обработка нажатия клавиши
  * @param  key: код нажатой клавиши
  * @retval None
  */
void keyboard_process_key(uint8_t key) {
    if (system_mode == 0) {
        // Нормальный режим - переход в меню
        if (key == KEY_ENTER) {
            system_mode = 1;  // Режим установки времени
            menu_position = 0;
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_print_string("Set Time:");
            lcd_set_cursor(1, 0);
            lcd_print_string("HH:MM:SS");
        }
        else if (key == KEY_ESC) {
            system_mode = 2;  // Режим установки расписания
            menu_position = 0;
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_print_string("Set Schedule:");
            lcd_set_cursor(1, 0);
            lcd_print_string("ON Time");
        }
    }
    else if (system_mode == 1) {
        // Режим установки времени
        switch (key) {
            case KEY_UP:
                if (menu_position == 0) current_time.hours = (current_time.hours + 1) % 24;
                else if (menu_position == 1) current_time.minutes = (current_time.minutes + 1) % 60;
                else if (menu_position == 2) current_time.seconds = (current_time.seconds + 1) % 60;
                break;
                
            case KEY_DOWN:
                if (menu_position == 0) current_time.hours = (current_time.hours + 23) % 24;
                else if (menu_position == 1) current_time.minutes = (current_time.minutes + 59) % 60;
                else if (menu_position == 2) current_time.seconds = (current_time.seconds + 59) % 60;
                break;
                
            case KEY_ENTER:
                menu_position = (menu_position + 1) % 3;
                break;
                
            case KEY_ESC:
                // Сохранение времени и возврат в нормальный режим
                RTC_SetDateTime(&current_time, &current_date);
                system_mode = 0;
                lcd_clear();
                break;
        }
        
        // Обновление отображения
        if (system_mode == 1) {
            lcd_set_cursor(1, 0);
            char buffer[16];
            sprintf(buffer, "%02d:%02d:%02d", 
                    current_time.hours, current_time.minutes, current_time.seconds);
            lcd_print_string(buffer);
            
            // Позиция курсора
            lcd_set_cursor(1, menu_position * 3);
        }
    }
    else if (system_mode == 2) {
        // Режим установки расписания
        static uint8_t schedule_mode = 0;  // 0: ON time, 1: OFF time, 2: Date
        
        switch (key) {
            case KEY_UP:
                if (schedule_mode == 0) {
                    if (menu_position == 0) device_schedule.on_time.hours = (device_schedule.on_time.hours + 1) % 24;
                    else if (menu_position == 1) device_schedule.on_time.minutes = (device_schedule.on_time.minutes + 1) % 60;
                    else if (menu_position == 2) device_schedule.on_time.seconds = (device_schedule.on_time.seconds + 1) % 60;
                }
                else if (schedule_mode == 1) {
                    if (menu_position == 0) device_schedule.off_time.hours = (device_schedule.off_time.hours + 1) % 24;
                    else if (menu_position == 1) device_schedule.off_time.minutes = (device_schedule.off_time.minutes + 1) % 60;
                    else if (menu_position == 2) device_schedule.off_time.seconds = (device_schedule.off_time.seconds + 1) % 60;
                }
                break;
                
            case KEY_DOWN:
                if (schedule_mode == 0) {
                    if (menu_position == 0) device_schedule.on_time.hours = (device_schedule.on_time.hours + 23) % 24;
                    else if (menu_position == 1) device_schedule.on_time.minutes = (device_schedule.on_time.minutes + 59) % 60;
                    else if (menu_position == 2) device_schedule.on_time.seconds = (device_schedule.on_time.seconds + 59) % 60;
                }
                else if (schedule_mode == 1) {
                    if (menu_position == 0) device_schedule.off_time.hours = (device_schedule.off_time.hours + 23) % 24;
                    else if (menu_position == 1) device_schedule.off_time.minutes = (device_schedule.off_time.minutes + 59) % 60;
                    else if (menu_position == 2) device_schedule.off_time.seconds = (device_schedule.off_time.seconds + 59) % 60;
                }
                break;
                
            case KEY_ENTER:
                schedule_mode = (schedule_mode + 1) % 3;
                menu_position = 0;
                if (schedule_mode == 0) {
                    lcd_set_cursor(1, 0);
                    lcd_print_string("ON Time    ");
                }
                else if (schedule_mode == 1) {
                    lcd_set_cursor(1, 0);
                    lcd_print_string("OFF Time   ");
                }
                else if (schedule_mode == 2) {
                    device_schedule.enabled = 1;
                    system_mode = 0;
                    lcd_clear();
                    uart_send_string("Schedule set and enabled\r\n");
                }
                break;
                
            case KEY_ESC:
                system_mode = 0;
                lcd_clear();
                break;
        }
        
        // Обновление отображения
        if (system_mode == 2) {
            lcd_set_cursor(1, 0);
            char buffer[16];
            if (schedule_mode == 0) {
                sprintf(buffer, "%02d:%02d:%02d", 
                        device_schedule.on_time.hours, 
                        device_schedule.on_time.minutes, 
                        device_schedule.on_time.seconds);
            }
            else if (schedule_mode == 1) {
                sprintf(buffer, "%02d:%02d:%02d", 
                        device_schedule.off_time.hours, 
                        device_schedule.off_time.minutes, 
                        device_schedule.off_time.seconds);
            }
            lcd_print_string(buffer);
        }
    }
}
