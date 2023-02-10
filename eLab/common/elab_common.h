/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ELAB_COMMON_H
#define ELAB_COMMON_H

/* includes ----------------------------------------------------------------- */
#include <stdint.h>

#if (ELAB_RTOS_CMSIS_OS_EN != 0)
#include "elab_port.h"
#include "cmsis_os.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* public typedef ----------------------------------------------------------- */
/* time related. */
uint32_t elab_time_ms(void);

/* uart debug related. */
void elab_debug_uart_init(uint32_t baudrate);
int16_t elab_debug_uart_send(void *buffer, uint16_t size);
int16_t elab_debug_uart_receive(void *buffer, uint16_t size);
void elab_debug_uart_buffer_clear(void);

#if (ELAB_RTOS_CMSIS_OS_EN != 0)
/* thread related. */
osThreadId_t elab_thread_init(elab_thread_t *const me,
                                void (*entry)(void *parameter),
                                const char *name, void *data,
                                void *stack, uint32_t stack_size,
                                uint8_t priority);
#endif

#ifdef __cplusplus
}
#endif

#endif

/* ----------------------------- end of file -------------------------------- */
