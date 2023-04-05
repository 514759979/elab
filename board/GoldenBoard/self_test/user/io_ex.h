#ifndef IO_EX_H
#define IO_EX_H

#include <stdbool.h>
#include <stdint.h>

void io_ex_set_out(uint8_t id, bool output);
void io_ex_set_status(uint8_t id, bool status);
bool io_ex_get_status(uint8_t id);

#endif
