/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef OLED_H
#define OLED_H

/* includes ----------------------------------------------------------------- */
#include <stdint.h>

/* public functions --------------------------------------------------------- */
void oled_open(void);
void oled_close(void);
void oled_clear(void);
void oled_set_value(uint8_t x, uint8_t y, uint8_t value);

#endif

/* ----------------------------- end of file -------------------------------- */
