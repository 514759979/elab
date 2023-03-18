#ifndef OLED_H
#define OLED_H

#include <stdint.h>
#include <stdbool.h>

void oled_init(void);
void oled_open(void);
void oled_close(void);
void oled_clear(void);
void oled_set_pos(uint8_t x, uint8_t y, bool status);

#endif
