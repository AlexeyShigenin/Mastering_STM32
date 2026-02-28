/**
	******************************************************************************
	* @file		matrix_keyboard.c
	* @brief	Опрос матричной клавиатуры
	*
	* PA3 - PA0 - Линии столбцов
	* PA4 - PA7 - Линии строк
	*
	*	PA3 - Столбец С0
	* PA2 - Столбец С1
	* PA1 - Столбец С2
	* PA0 - Столбец С3
	* PA4 - Строка R0
	* PA5 - Строка R1
	* PA6 - Строка R2
	* PA7 - Строка R3
	******************************************************************************
	*/

#include "matrix_keyboard.h"
#include "lcd.h"
#include "rtc.h"


/* Внешние переменные */
extern RTCTimeDate currentTime;					// структура определена в rtc.c
extern ScheduleTypeDef deviceSchedule;	// структура определена в rtc.c

#define BOUNCE	20							// константа антидребезга
#define LONG_TRESHOLD 100000		// Порог для определения длительного нажатия (100000 если без задержек в main, 500 если задержка 5ms)
#define SHORT_TRESHOLD 100			// Порог для определения короткого нажатия	(100 если без задержек в main, 5 если задержка 5ms)

// Состояния
enum {
    STATE_DISPLAY,          // отображение (циклическое переключение экранов)
    STATE_SET_TIME,         // установка текущего времени/даты
    STATE_SET_SCHEDULE      // установка расписания
};

// Подсостояния для установки времени
enum {
    TIME_EDIT_TIME,         // редактирование времени
    TIME_EDIT_DATE          // редактирование даты
};

// Подсостояния для установки расписания
enum {
    SCHEDULE_EDIT_ON_TIME,
    SCHEDULE_EDIT_ON_DATE,
    SCHEDULE_EDIT_OFF_TIME,
    SCHEDULE_EDIT_OFF_DATE
};

// Текущее состояние
uint8_t currentState = STATE_DISPLAY;
uint8_t displayPage = 0;          // 0 - текущее время, 1 - расписание ON, 2 - расписание OFF

// Для режима установки времени
static uint8_t setTimeSubmode = TIME_EDIT_TIME;
static uint8_t TimeEditPos = 0;         // 0-2: часы, минуты, секунды; или день, месяц, год
static RTCTimeDate tempTime;

// Для режима установки расписания
static uint8_t scheduleSubmode = SCHEDULE_EDIT_ON_TIME;
static uint8_t scheduleEditPos = 0;
static ScheduleTypeDef scheduleTempTime;

// Вспомогательные функции обновления экрана
static void displayUpdate(void);
static void displaySetTime(void);
static void displaySetSchedule(void);

// Массив символов на клавиатуре для конвертации
int keyb [4][4] =
{
  {1, 2, 3, 4},
  {5, 6, 7, 8},
  {9, 10, 11, 12},
  {13, 14, 15, 16}
};

/* Коды кнопок (+100 - длительное нажатие) */
#define KEY_NONE					-1
#define KEY_UP						2
#define KEY_DOWN					10
#define KEY_ENTER					6
#define KEY_ENTER_LONG		106
#define KEY_LEFT					5
#define KEY_RIGHT					7
#define KEY_SET_TIME			104
#define KEY_SET_SCHEDULE	108
#define KEY_ESC						16


/**
	******************************************************************************
	* @brief	Инициализация порта клавиатуры
	* @param	None
	* @retval None
	*/
void keyboardInit(void){
	// Включение тактирования порта А
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

	// PA0-PA3 столбцы -	output, open drain (CNFx_0=1, CNFx_1=0),
	//										2 MHz (MODEx_0=0, MODEx_1=1)
		GPIOA->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0 |
									GPIO_CRL_CNF1 | GPIO_CRL_MODE1 |
									GPIO_CRL_CNF2 | GPIO_CRL_MODE2 |
									GPIO_CRL_CNF3 | GPIO_CRL_MODE3);
	GPIOA->CRL |= GPIO_CRL_CNF0_0 | GPIO_CRL_MODE0_1 |
								GPIO_CRL_CNF1_0 | GPIO_CRL_MODE1_1 |
								GPIO_CRL_CNF2_0 | GPIO_CRL_MODE2_1 |
								GPIO_CRL_CNF3_0 | GPIO_CRL_MODE3_1;
	
	// PA4-PA7 строки - input, pull up
	// (CNFx_0=0, CNFx_1=1, MODEx_0=0, MODEx_1=0, PxODR=1)
	GPIOA->CRL &= ~(GPIO_CRL_CNF4 | GPIO_CRL_MODE4 |
									GPIO_CRL_CNF5 | GPIO_CRL_MODE5 |
									GPIO_CRL_CNF6 | GPIO_CRL_MODE6 |
									GPIO_CRL_CNF7 | GPIO_CRL_MODE7);
	GPIOA->CRL |= GPIO_CRL_CNF4_1 | GPIO_CRL_CNF5_1 |
								GPIO_CRL_CNF6_1 | GPIO_CRL_CNF7_1;
	
	// Подтяжка к питанию - pull up (PxODR=1):
	GPIOA->ODR |= GPIO_ODR_ODR4 | 
								GPIO_ODR_ODR5 |
								GPIO_ODR_ODR6 | 
								GPIO_ODR_ODR7;
}

/**
	******************************************************************************
	* @brief	Установка логического 0 на выбранном стобце
	* @param	col		Выбранный столбец
	* @retval None
	*/
void setColumn(uint8_t col){
	// Сброс линий столбцов (установка на всех столбцах логической 1
	GPIOA->BSRR |= GPIO_BSRR_BS0 | GPIO_BSRR_BS1 | GPIO_BSRR_BS2 | GPIO_BSRR_BS3;
	// Установка логического 0 на выбранном столбце
	switch(col){
		case 0: GPIOA->BSRR |= GPIO_BSRR_BR3; return;
		case 1: GPIOA->BSRR |= GPIO_BSRR_BR2; return;
		case 2: GPIOA->BSRR |= GPIO_BSRR_BR1; return;
		case 3: GPIOA->BSRR |= GPIO_BSRR_BR0; return;
	}
}

/**
	******************************************************************************
	* @brief	Опрос линий строк
	* @param	None
	* @retval	Номер строки, на которой установлен логический 0
	*/
// Считывание линии строк
int readString(){														
	if ((GPIOA->IDR & GPIO_IDR_IDR4) == 0) return 0;	// логический 0 на строке 0
	if ((GPIOA->IDR & GPIO_IDR_IDR5) == 0) return 1;	// логический 0 на строке 1
	if ((GPIOA->IDR & GPIO_IDR_IDR6) == 0) return 2;	// логический 0 на строке 2
	if ((GPIOA->IDR & GPIO_IDR_IDR7) == 0) return 3;	// логический 0 на строке 3
	return KEY_NONE; 																	// ни на одной строке нет 0
}

/**
	******************************************************************************
	* @brief	Опрос клавиатуры
	* @param	None
	* @retval	Код нажатой кнопки
	*/

int scanKeyboard(void) {
	static int bounce = 0;								// счетчик дребезга
	int btn;															// считанное значение кнопки
	static int btnOld = 0;								// предыдущее состояние кнопки
		
	// Опрос клавиатуры
	for (int col = 0; col < 4; col++) {		// Опрос клавиатуры
		setColumn(col);											// Сброс уровня на столбце
		delayDWT_us(2);
		btn = readString();									// Считывание строки
		if (btn != KEY_NONE)	{							// Проверка на нажатие
			btn = keyb[btn][col];							// Конвертация в символы
			break;														// выход из цикла опроса
		}
	}
	
	// Антидребезг
	if (btn == KEY_NONE || btnOld != btn) {	// Если кнопка не нажата или изменилось состояние кнопки
		btnOld = btn;													// Записываем прошлое состояние кнопки
		bounce = 0;														// Обнуляем счеткик дребезга
		return KEY_NONE;											// Возвращвем признак ненажатой кнопки
	}

	if (bounce == BOUNCE){									// Если счетчик досчитал до константы дребезга
		//bounce++;														// Одиночный возврат считанного значения
		return btn;														// Возвращвем код нажатой кнопки
	}

	if (bounce < BOUNCE) bounce++;					// Если счетчик дребезга не заполнен

	return KEY_NONE;												// Возвращвем признак ненажатой кнопки
}

/**
	* @brief	Определение длительности нажатия
	* @param	None
	* @retval	Код нажатой кнопки
	*
	* Функция получения кода нажатой кнопки с поддержкой длительных нажатий.
	* При каждом вызове опрашивает клавиатуру и возвращает:
	*   - код кнопки (1..16) – если нажата обычная кнопка >= SHORT_TRESHOLD раз подряд;
	*   - код кнопки + 100 – если одна и та же кнопка нажата >= LONG_TRESHOLD раз подряд;
	*   - -1 – если ни одна кнопка не нажата (длительное отсутствие нажатий игнорируется).
	*/
int getKeyPress(void) {
	static int prev_btn = -2;								// предыдущее значение (заведомо не -1)
	static uint32_t countShort = 0;					// счётчик для короткого нажатия
	static uint32_t countLong = 0;					// счётчик для длительного нажатия
	static int longHandled = 0;							// флаг: было ли уже длительное нажатие для текущей кнопки
	static int shortHandled = 0;						// флаг: было ли уже короткое нажатие для текущей кнопки
	
	int btn = scanKeyboard();								// опрос клавиатуры
	
	// 1. Кнопка не нажата – сброс всего состояния
	if (btn == KEY_NONE) {
		prev_btn = KEY_NONE;
		countShort = 0;
		countLong = 0;
		longHandled = 0;
		shortHandled = 0;
		return KEY_NONE;
	}
	
	// 2. Нажата новая кнопка (или после отпускания)
	if (btn != prev_btn) {
		prev_btn = btn;
		countShort = 1;
		countLong = 1;
		longHandled = 0;
		shortHandled = 0;
		return KEY_NONE;
	}
	// 3. Та же кнопка удерживается
	if (shortHandled) {
		// Короткое обработано
		if (longHandled) {
			// Короткое и длинное обработано– игнорируем дальнейшие удержания
			return KEY_NONE;
		} 
		// Только короткое обработано
		countLong++;
		// 4. Проверка порога длительного нажатия
		if (countLong >= LONG_TRESHOLD) {
			longHandled = 1;										// помечаем, что длительное обработано
			return btn + 100;										// код длительного нажатия
		}
		return KEY_NONE;
	}
	// Короткое нажатие еще не обработано - увеличиваем счётчик
	countShort++;
	
	// 5. Проверка порога короткого нажатия
	if (countShort >= SHORT_TRESHOLD) {
		shortHandled = 1;											// помечаем, что короткое обработано
		return btn;														// обычный код кнопки
		
	}
	// 6. Нажатие короче обоих порогов
	return KEY_NONE;
}

void ui_init(void) {
    currentState = STATE_DISPLAY;
    displayPage = 0;
    displayUpdate();
}

/**
	******************************************************************************
  * @brief  Обработка нажатия клавиши
  * @param  key: код нажатой клавиши
  * @retval None
  */
void keyboardProcessKey(int key) {
	switch (currentState) {
        case STATE_DISPLAY:
            if (key == KEY_UP || key == KEY_DOWN) {
                // Переключение страниц: 0->1->2->0
                displayPage = (displayPage + 1) % 3;
                displayUpdate();
            }
            else if (key == KEY_SET_TIME) {
                // Вход в установку времени
                currentState = STATE_SET_TIME;
                setTimeSubmode = TIME_EDIT_TIME;
                TimeEditPos = 0;
                tempTime = currentTime;
                displaySetTime();
            }
            else if (key == KEY_SET_SCHEDULE) {
                // Вход в установку расписания
                currentState = STATE_SET_SCHEDULE;
                scheduleSubmode = SCHEDULE_EDIT_ON_TIME;
                scheduleEditPos = 0;
                scheduleTempTime = deviceSchedule;
                displaySetSchedule();
            }
            // Остальные кнопки игнорируем
            break;

        case STATE_SET_TIME:
            if (key == KEY_ESC) {
                // Выход без сохранения
                currentState = STATE_DISPLAY;
                displayUpdate();
                break;
            }
            else if (key == KEY_ENTER_LONG) {
                // Сохранить изменения
                currentTime = tempTime;
                RTCSetTimeDate(&currentTime);
                currentState = STATE_DISPLAY;
                displayUpdate();
                break;
            }
            else if (key == KEY_ENTER) {
                // Переключение между временем и датой
                setTimeSubmode = (setTimeSubmode == TIME_EDIT_TIME) ? TIME_EDIT_DATE : TIME_EDIT_TIME;
                TimeEditPos = 0;
                displaySetTime();
                break;
            }

            // Обработка LEFT/RIGHT для перемещения по полям
            if (key == KEY_LEFT || key == KEY_RIGHT) {
                if (setTimeSubmode == TIME_EDIT_TIME) {
                    // Поля: часы, минуты, секунды
                    if (key == KEY_RIGHT) {
                        TimeEditPos = (TimeEditPos + 1) % 3;
                    } else {
                        TimeEditPos = (TimeEditPos + 2) % 3;
                    }
                } else {
                    // Поля даты: день, месяц, год
                    if (key == KEY_RIGHT) {
                        TimeEditPos = (TimeEditPos + 1) % 3;
                    } else {
                        TimeEditPos = (TimeEditPos + 2) % 3;
                    }
                }
                displaySetTime();
                break;
            }

            // Обработка UP/DOWN для изменения значения
            if (key == KEY_UP || key == KEY_DOWN) {
                int delta = (key == KEY_UP) ? 1 : -1;
                if (setTimeSubmode == TIME_EDIT_TIME) {
                    switch (TimeEditPos) {
                        case 0: // часы
                            tempTime.hours = (tempTime.hours + delta + 24) % 24;
                            break;
                        case 1: // минуты
                            tempTime.minutes = (tempTime.minutes + delta + 60) % 60;
                            break;
                        case 2: // секунды
                            tempTime.seconds = (tempTime.seconds + delta + 60) % 60;
                            break;
                    }
                } else {
                    switch (TimeEditPos) {
                        case 0: // день
                            // Без проверки корректности
                            tempTime.day = (tempTime.day + delta + 31) % 31;
                            if (tempTime.day == 0) tempTime.day = 31;
                            break;
                        case 1: // месяц
                            tempTime.month = (tempTime.month + delta + 12) % 12;
                            if (tempTime.month == 0) tempTime.month = 12;
                            break;
                        case 2: // год
                            tempTime.year = (tempTime.year + delta);
                            break;
                    }
                }
                displaySetTime();
                break;
            }
            break;

        case STATE_SET_SCHEDULE:
            if (key == KEY_ESC) {
                // Выход без сохранения
                currentState = STATE_DISPLAY;
                displayUpdate();
                break;
            }
            else if (key == KEY_ENTER_LONG) {
                // Сохранить все изменения расписания
                deviceSchedule = scheduleTempTime;
							schedulerSetOnTime(&deviceSchedule.onTime);
							schedulerSetOffTime(&deviceSchedule.offTime);
                currentState = STATE_DISPLAY;
                displayUpdate();
                break;
            }
            else if (key == KEY_ENTER) {
                // Переключение между подрежимами: ON_TIME -> ON_DATE -> OFF_TIME -> OFF_DATE -> ON_TIME
                scheduleSubmode = (scheduleSubmode + 1) % 4;
                scheduleEditPos = 0;
                displaySetSchedule();
                break;
            }

            if (key == KEY_LEFT || key == KEY_RIGHT) {
                // Определяем количество полей для текущего подрежима
                uint8_t max_pos = 0;
                if (scheduleSubmode == SCHEDULE_EDIT_ON_TIME || scheduleSubmode == SCHEDULE_EDIT_OFF_TIME) {
                    max_pos = 3; // часы, минуты, секунды
                } else {
                    max_pos = 3; // день, месяц, год
                }
                if (key == KEY_RIGHT) {
                    scheduleEditPos = (scheduleEditPos + 1) % max_pos;
                } else {
                    scheduleEditPos = (scheduleEditPos + max_pos - 1) % max_pos;
                }
                displaySetSchedule();
                break;
            }

            if (key == KEY_UP || key == KEY_DOWN) {
                int delta = (key == KEY_UP) ? 1 : -1;
                // В зависимости от подрежима изменяем нужную временную переменную
                switch (scheduleSubmode) {
                    case SCHEDULE_EDIT_ON_TIME:
                        switch (scheduleEditPos) {
                            case 0: scheduleTempTime.onTime.hours   = (scheduleTempTime.onTime.hours + delta + 24) % 24; break;
                            case 1: scheduleTempTime.onTime.minutes = (scheduleTempTime.onTime.minutes + delta + 60) % 60; break;
                            case 2: scheduleTempTime.onTime.seconds = (scheduleTempTime.onTime.seconds + delta + 60) % 60; break;
                        }
                        break;
                    case SCHEDULE_EDIT_ON_DATE:
                        switch (scheduleEditPos) {
                            case 0:
                                scheduleTempTime.onTime.day = (scheduleTempTime.onTime.day + delta + 31) % 31;
                                if (scheduleTempTime.onTime.day == 0) scheduleTempTime.onTime.day = 31;
                                break;
                            case 1:
                                scheduleTempTime.onTime.month = (scheduleTempTime.onTime.month + delta + 12) % 12;
                                if (scheduleTempTime.onTime.month == 0) scheduleTempTime.onTime.month = 12;
                                break;
                            case 2:
                                scheduleTempTime.onTime.year = (scheduleTempTime.onTime.year+ 1);
                                break;
                        }
                        break;
                    case SCHEDULE_EDIT_OFF_TIME:
                        switch (scheduleEditPos) {
                            case 0: scheduleTempTime.offTime.hours   = (scheduleTempTime.offTime.hours + delta + 24) % 24; break;
                            case 1: scheduleTempTime.offTime.minutes = (scheduleTempTime.offTime.minutes + delta + 60) % 60; break;
                            case 2: scheduleTempTime.offTime.seconds = (scheduleTempTime.offTime.seconds + delta + 60) % 60; break;
                        }
                        break;
                    case SCHEDULE_EDIT_OFF_DATE:
                        switch (scheduleEditPos) {
                            case 0:
                                scheduleTempTime.offTime.day = (scheduleTempTime.offTime.day + delta + 31) % 31;
                                if (scheduleTempTime.offTime.day == 0) scheduleTempTime.offTime.day = 31;
                                break;
                            case 1:
                                scheduleTempTime.offTime.month = (scheduleTempTime.offTime.month + delta + 12) % 12;
                                if (scheduleTempTime.offTime.month == 0) scheduleTempTime.offTime.month = 12;
                                break;
                            case 2:
                                scheduleTempTime.offTime.year = (scheduleTempTime.offTime.year + delta);
                                break;
                        }
                        break;
                }
                displaySetSchedule();
                break;
            }
            break;
    }
}

// Обновление дисплея в режиме отображения
static void displayUpdate(void) {
    lcdClear();
		lcdCursorOff();
    char buf[17];
    switch (displayPage) {
        case 0: // текущее время
					// Экран текущего времени формируется в lcd.c
            break;
        case 1: // время включения
            lcdSetCursor(0, 0);
            sprintf(buf, "ON: %02d/%02d/%04d", deviceSchedule.onTime.day, deviceSchedule.onTime.month, deviceSchedule.onTime.year);
            lcdPrintString(buf);
            lcdSetCursor(1, 4);
            sprintf(buf, "%02d:%02d:%02d", deviceSchedule.onTime.hours, deviceSchedule.onTime.minutes, deviceSchedule.onTime.seconds);
            lcdPrintString(buf);
            break;
        case 2: // время выключения
            lcdSetCursor(0, 0);
            sprintf(buf, "OFF:%02d/%02d/%04d", deviceSchedule.offTime.day, deviceSchedule.offTime.month, deviceSchedule.offTime.year);
            lcdPrintString(buf);
            lcdSetCursor(1,4);
            sprintf(buf, "%02d:%02d:%02d", deviceSchedule.offTime.hours, deviceSchedule.offTime.minutes, deviceSchedule.offTime.seconds);
            lcdPrintString(buf);
            break;
    }
}

// Отображение в режиме установки времени
static void displaySetTime(void) {
    lcdClear();
    char buf[17];
    if (setTimeSubmode == TIME_EDIT_TIME) {
			lcdCursorOn();
        lcdSetCursor(0, 0);
        lcdPrintString("SET TIME");
        lcdSetCursor(1, 0);
        sprintf(buf, "%02d:%02d:%02d", tempTime.hours, tempTime.minutes, tempTime.seconds);
        lcdPrintString(buf);
        // Установка курсора
        lcdSetCursor(1, TimeEditPos * 3);
    } else {
			lcdCursorOn();
        lcdSetCursor(0, 0);
        lcdPrintString("SET DATE");
        lcdSetCursor(1, 0);
        sprintf(buf, "%02d/%02d/%04d", tempTime.day, tempTime.month, tempTime.year);
        lcdPrintString(buf);
        // Позиции: день (0), месяц (3), год (6)
        lcdSetCursor(1, TimeEditPos * 3);
    }
}

// Отображение в режиме установки расписания
static void displaySetSchedule(void) {
    lcdClear();
    char buf[17];
    switch (scheduleSubmode) {
        case SCHEDULE_EDIT_ON_TIME:
					lcdCursorOn();
            lcdSetCursor(0, 0);
            lcdPrintString("SET ON TIME");
            lcdSetCursor(1, 0);
            sprintf(buf, "%02d:%02d:%02d", scheduleTempTime.onTime.hours, scheduleTempTime.onTime.minutes, scheduleTempTime.onTime.seconds);
            lcdPrintString(buf);
            lcdSetCursor(1, scheduleEditPos * 3);
            break;
        case SCHEDULE_EDIT_ON_DATE:
					lcdCursorOn();
            lcdSetCursor(0, 0);
            lcdPrintString("SET ON DATE");
            lcdSetCursor(1, 0);
            sprintf(buf, "%02d/%02d/%04d", scheduleTempTime.onTime.day, scheduleTempTime.onTime.month, scheduleTempTime.onTime.year);
            lcdPrintString(buf);
            lcdSetCursor(1, scheduleEditPos * 3);
            break;
        case SCHEDULE_EDIT_OFF_TIME:
					lcdCursorOn();
            lcdSetCursor(0, 0);
            lcdPrintString("SET OFF TIME");
            lcdSetCursor(1, 0);
            sprintf(buf, "%02d:%02d:%02d", scheduleTempTime.offTime.hours, scheduleTempTime.offTime.minutes, scheduleTempTime.offTime.seconds);
            lcdPrintString(buf);
            lcdSetCursor(1, scheduleEditPos * 3);
            break;
        case SCHEDULE_EDIT_OFF_DATE:
					lcdCursorOn();
            lcdSetCursor(0, 0);
            lcdPrintString("SET OFF DATE");
            lcdSetCursor(1, 0);
            sprintf(buf, "%02d/%02d/%04d", scheduleTempTime.offTime.day, scheduleTempTime.offTime.month, scheduleTempTime.offTime.year);
            lcdPrintString(buf);
            lcdSetCursor(1, scheduleEditPos * 3);
            break;
    }
}
