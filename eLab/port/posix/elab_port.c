/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "elab_port.h"
#include "elab_common.h"
#include "elab_assert.h"
#include "cmsis_os.h"
#include "elab_def.h"
#include "elab_assert.h"

ELAB_TAG("eLabPortPOSIX");

/* private define ----------------------------------------------------------- */
#define ELAB_DEBUG_BUFF_SIZE                    (2048)

/* private variables -------------------------------------------------------- */
static char buff_tx[ELAB_DEBUG_BUFF_SIZE];

/* public function ---------------------------------------------------------- */
/**
  * @brief  eLab initialization exporting function.
  * @param  level       init level.
  * @retval None
  */
uint32_t elab_time_ms(void)
{
#if (ELAB_RTOS_CMSIS_OS_EN != 0)
    return osKernelSysTick();
#else
    return 0;
#endif
}

void *elab_malloc(uint32_t size)
{
    return malloc(size);
}

void elab_free(void *memory)
{
    return free(memory);
}

int16_t elab_debug_uart_send(void *buffer, uint16_t size)
{
    elab_assert(size < ELAB_DEBUG_BUFF_SIZE);

    memcpy(buff_tx, buffer, size);
    buff_tx[size] = 0;

    printf("%s", buff_tx);
    fflush(stdout);

    return 0;
}

/**
  * @brief  eLab thread initialization function.
  * @retval None
  */
#if (ELAB_RTOS_CMSIS_OS_EN != 0)
osThreadId_t elab_thread_init(elab_thread_t *const me,
                                void (*entry)(void *parameter),
                                const char *name, void *data,
                                void *stack, uint32_t stack_size,
                                uint8_t priority)
{
    (void)me;

    osThreadAttr_t thread_attr;
    thread_attr.name = name;
    thread_attr.stack_size = stack_size;

    osThreadId_t thread = osThreadNew(entry, data, &thread_attr);

    elab_assert(thread != (osThreadId_t)0);

    return thread;
}
#endif

/* ----------------------------- end of file -------------------------------- */
