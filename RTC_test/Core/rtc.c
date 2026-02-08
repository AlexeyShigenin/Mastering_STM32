/**
	******************************************************************************
	* @file			rtc.c
	* @brief		Функции для работы с RTC
	******************************************************************************
	*/
																																	
#include "stm32f10x.h"                  // Device header
#include "lcd.h"

RTCTimeDate currentTime;
/* Глобальная структура для хранения расписания */
ScheduleTypeDef deviceSchedule = {0};

// Инициализация источника тактирования RTC
void RTCInitClockSource(void) {
    /* Настраивает источник тактирования RTC (LSE) */
    
    // Включение LSE (Low Speed External) кварца 32.768 kHz
    RCC->BDCR |= RCC_BDCR_LSEON;
    
    // Ожидание стабилизации LSE (может занять до 2 секунд)
    uint32_t timeout = 0xFFFFF;	// Переменная для исключения зависаний (например, отсутствие кварца)
    while(!(RCC->BDCR & RCC_BDCR_LSERDY) && timeout--){
			if (timeout == 0) {
				// LSE не запустился - возможная ошибка
				// Здесь можно переключиться на LSI или сгенерировать ошибку
				break;
		};
	}
    
    // Выбор LSE в качестве источника тактирования RTC
    // RTCSEL[1:0] = 01: LSE selected
    RCC->BDCR &= ~RCC_BDCR_RTCSEL;          // Сброс битов выбора источника
    RCC->BDCR |= RCC_BDCR_RTCSEL_LSE;       // Выбор LSE
    
    // Включение тактирования RTC
    RCC->BDCR |= RCC_BDCR_RTCEN;
}

// Инициализация RTC с учетом предыдущей инициализации
void rtcInit(void) {
    // Основная функция инициализации RTC действует исходя из проверки, был ли RTC уже инициализирован
    
    // 1. Включение тактирования Power Interface и Backup Interface
    RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;
    
    // 2. Разрешение доступа к Domain Backup
    PWR->CR |= PWR_CR_DBP;
    
    // 3. Проверка, был ли RTC уже инициализирован
    if(!(BKP->DR1 == 0x5A5A)) {
        /* RTC не был инициализирован - выполняем полную инициализацию */
        
        // 3.1. Сброс Backup Domain (при первом включении)
        RCC->BDCR |= RCC_BDCR_BDRST;     // Установка бита сброса
        RCC->BDCR &= ~RCC_BDCR_BDRST;    // Сброс бита сброса
        
        // 3.2. Инициализация источника тактирования
        RTCInitClockSource();
        
        // 3.3. Настройка предделителя для получения 1 Гц
        // При частоте LSE = 32768 Гц, для получения 1 Гц нужен предделитель 32768-1
        // PRL = 32768 - 1 = 32767 = 0x7FFF
        
        // Ожидание завершения предыдущей операции с RTC
        while(!(RTC->CRL & RTC_CRL_RTOFF));
        
        // Вход в режим конфигурации
        RTC->CRL |= RTC_CRL_CNF;
        
        // Установка предделителя
        // PRLH содержит старшие 4 бита 20-битного значения
        // PRLL содержит младшие 16 бит
        RTC->PRLH = (0x7FFF >> 16) & 0x0F;  // 0x0000
        RTC->PRLL = 0x7FFF & 0xFFFF;        // 0x7FFF
        
        // Выход из режима конфигурации
        RTC->CRL &= ~RTC_CRL_CNF;
        
        // Ожидание завершения операции
        while(!(RTC->CRL & RTC_CRL_RTOFF));
        
        // 3.4. Установка начального времени (опционально)
        RTCTimeDate initialTime = {
        .seconds = 0,
        .minutes = 27,
        .hours = 23,
        .day = 23,
        .month = 1,
        .year = 2026,
        .weekday = 5
    };
        RTCSetTimeDate(&initialTime);
        
        // 3.5. Установка флага инициализации в резервный регистр
        BKP->DR1 = 0x5A5A;  // Флаг успешной инициализации
        
        // 3.6. Сброс всех флагов прерываний RTC
        RTC->CRL &= ~(RTC_CRL_SECF | RTC_CRL_ALRF | RTC_CRL_OWF);
    }
    else {
        /* RTC уже был инициализирован - проверяем и корректируем настройки */
        
//        // 3.7. Проверка, запущен ли RTC
//        if(!(RCC->BDCR & RCC_BDCR_RTCEN)) {
//            // RTC отключен - включаем его
//            RCC->BDCR |= RCC_BDCR_RTCEN;
//        }
//        
//        // 3.8. Проверка источника тактирования
//        uint32_t rtcsel = RCC->BDCR & RCC_BDCR_RTCSEL;
//        if(rtcsel != RCC_BDCR_RTCSEL_LSE) {
//            // Источник тактирования не LSE - переключаем
//            // Сначала отключаем RTC
//            RCC->BDCR &= ~RCC_BDCR_RTCEN;
//            
//            // Выбираем LSE
//            RCC->BDCR &= ~RCC_BDCR_RTCSEL;
//            RCC->BDCR |= RCC_BDCR_RTCSEL_LSE;
//            
//            // Включаем RTC
//            RCC->BDCR |= RCC_BDCR_RTCEN;
//        }
//        
//        // 3.9. Проверка и запуск LSE если необходимо
//        if(!(RCC->BDCR & RCC_BDCR_LSEON)) {
//            RCC->BDCR |= RCC_BDCR_LSEON;
//            
//            // Ожидание стабилизации LSE
//            uint32_t timeout = 0xFFFFF;
//            while(!(RCC->BDCR & RCC_BDCR_LSERDY)) {
//                timeout--;
//                if(timeout == 0) break;
//            }
//        }
//        
//        // 3.10. Синхронизация с RTC
//        RTC->CRL &= ~RTC_CRL_RSF;
//        while(!(RTC->CRL & RTC_CRL_RSF));
    }
    
    // 4. Настройка прерываний RTC (всегда, т.к. при сбросе основного ядра настройки прерываний сбрасываются)
    
    // 4.1. Ожидаем синхронизации регистров
    RTC->CRL &= ~RTC_CRL_RSF;
    while(!(RTC->CRL & RTC_CRL_RSF));
    
    // 4.2. Включение прерывания по секундам
    RTC->CRH |= RTC_CRH_SECIE;
    
    // 4.3. Настройка приоритета прерываний в NVIC
    NVIC_SetPriority(RTC_IRQn, 0x0F);  // Низкий приоритет
    NVIC_EnableIRQ(RTC_IRQn);          // Включение прерывания RTC
    
    // 5. (Опционально) Запрет доступа к Backup Domain
    // PWR->CR &= ~PWR_CR_DBP;
}

// Обработчик прерывания RTC
void RTC_IRQHandler(void) {
    // Проверка флага секунды
    if(RTC->CRL & RTC_CRL_SECF) {
			// Здесь можно обновить отображение времени
			//RTCGetTimeDate(&currentTime);	// Получение текущего времени
			//lcdUpdateTime(&currentTime);	// Обновление времени на индикаторе
			
        // Сброс флага секунды
        RTC->CRL &= ~RTC_CRL_SECF;
    }
    
    // Проверка флага будильника
    if(RTC->CRL & RTC_CRL_ALRF) {
        // Действия по будильнику
        
        // Сброс флага будильника
        RTC->CRL &= ~RTC_CRL_ALRF;
    }
    
    // Проверка флага переполнения
    if(RTC->CRL & RTC_CRL_OWF) {
        // Обработка переполнения счетчика (каждые ~136 лет)
        
        // Сброс флага переполнения
        RTC->CRL &= ~RTC_CRL_OWF;
    }
}

// Установка времени и даты
void RTCSetTimeDate(RTCTimeDate *td) {
    // Конвертация времени и даты в секунды
    uint32_t seconds = RTCConvertToSeconds(td);
    
    // Ожидание готовности RTC к записи
    while(!(RTC->CRL & RTC_CRL_RTOFF));
    
    // Вход в режим конфигурации
    RTC->CRL |= RTC_CRL_CNF;
    
    // Запись значения в счетчик
    RTC->CNTH = (seconds >> 16) & 0xFFFF;
    RTC->CNTL = seconds & 0xFFFF;
    
    // Выход из режима конфигурации
    RTC->CRL &= ~RTC_CRL_CNF;
    
    // Ожидание завершения операции
    while(!(RTC->CRL & RTC_CRL_RTOFF));
}

// Получение текущего времени и даты
void RTCGetTimeDate(RTCTimeDate *td) {
    uint32_t seconds;
    
    // Ожидание синхронизации регистров
    RTC->CRL &= ~RTC_CRL_RSF;
    while(!(RTC->CRL & RTC_CRL_RSF));
    
    // Чтение счетчика (два чтения для атомарности)
    do {
        seconds = (RTC->CNTH << 16) | RTC->CNTL;
    } while(seconds != ((RTC->CNTH << 16) | RTC->CNTL));
    
    // Конвертация секунд во время и дату
    RTCConvertFromSeconds(seconds, td);
}

// Проверка високосного года
uint8_t RTCIsLeapYear(uint16_t year) {
    // Год високосный, если:
    // 1. Делится на 400, ИЛИ
    // 2. Делится на 4 И НЕ делится на 100
    return ((year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0)));
}

// Конвертация времени и даты в секунды
uint32_t RTCConvertToSeconds(RTCTimeDate *td) {
    uint32_t totalDays = 0;
    uint16_t year;
    uint8_t month;
    
    // Расчет дней до начала текущего года
    for(year = 1970; year < td->year; year++) {
        totalDays += RTCIsLeapYear(year) ? 366 : 365;
    }
    
    // Массив дней в месяцах для невисокосного года
    const uint8_t daysInMonth[] = {31, 28, 31, 30, 31, 30, 
                                     31, 31, 30, 31, 30, 31};
    
    // Расчет дней в текущем году до начала текущего месяца
    for(month = 1; month < td->month; month++) {
        totalDays += daysInMonth[month - 1];
        
        // Коррекция для февраля в високосном году
        if(month == 2 && RTCIsLeapYear(td->year)) {
            totalDays += 1;
        }
    }
    
    // Добавление дней текущего месяца (день-1, т.к. с 1 числа)
    totalDays += (td->day - 1);
    
    // Расчет общего количества секунд
    uint32_t totalSeconds = totalDays * 86400UL;  // секунд в дне
    totalSeconds += td->hours * 3600UL;
    totalSeconds += td->minutes * 60UL;
    totalSeconds += td->seconds;
    
    return totalSeconds;
}

// Конвертация секунд во время и дату
void RTCConvertFromSeconds(uint32_t seconds, RTCTimeDate *td) {
    uint32_t days, remainingSeconds;
    uint16_t year = 1970;
    uint8_t month;
    uint8_t dayOfWeek;
    
    // Расчет количества дней
    days = seconds / 86400UL;
    remainingSeconds = seconds % 86400UL;
    
    // Расчет дня недели (1 января 1970 был четверг)
    dayOfWeek = (days + 4) % 7;  // 0=воскресенье, 1=понедельник...
    td->weekday = (dayOfWeek == 0) ? 7 : dayOfWeek;  // 1-пн, 7-вс
    
    // Расчет года
    while(1) {
        uint16_t daysInYear = RTCIsLeapYear(year) ? 366 : 365;
        
        if(days < daysInYear) {
            break;
        }
        
        days -= daysInYear;
        year++;
    }
    
    td->year = year;
    
    // Массив дней в месяцах для невисокосного года
    const uint8_t daysInMonth[] = {31, 28, 31, 30, 31, 30, 
																		31, 31, 30, 31, 30, 31};
    
    // Коррекция дней для високосного года
    uint8_t daysInFeb = RTCIsLeapYear(year) ? 29 : 28;
    
    // Расчет месяца
    for(month = 1; month <= 12; month++) {
        uint8_t daysThisMonth = (month == 2) ? daysInFeb : daysInMonth[month - 1];
        
        if(days < daysThisMonth) {
            break;
        }
        
        days -= daysThisMonth;
    }
    
    td->month = month;
    td->day = days + 1;  // +1 потому что дни считались с 0
    
    // Расчет времени
    td->hours = remainingSeconds / 3600;
    remainingSeconds %= 3600;
    td->minutes = remainingSeconds / 60;
    td->seconds = remainingSeconds % 60;
}




