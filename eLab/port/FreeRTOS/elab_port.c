/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include "elab.h"

/* public function ---------------------------------------------------------- */

/**
  * @brief  eLab initialization exporting function.
  * @param  level       init level.
  * @retval None
  */
uint32_t elab_time_ms(void)
{
#if (ELAB_RTOS_CMSIS_OS_EN != 0)
    return xTaskGetTickCount() * ELAB_RTOS_TICK_MS;
#elif (ELAB_RTOS_BASIC_OS_EN != 0)
    return bos_time();
#else
    return 0;
#endif
}

/**
  * @brief  eLab thread initialization function.
  * @retval None
  */
#if (ELAB_RTOS_CMSIS_OS_EN != 0)
osThreadId_t elab_thread_init(elab_thread_t *const me,
                                void (*entry)(void *parameter),
                                const char *name, void *data,
                                void *stack, uint32_t stack_size, uint8_t priority)
{
    TaskHandle_t thread;

    thread = xTaskCreateStatic(
                entry,                                  /* the task function */
                name ,                                  /* the name of the task */
                (stack_size / sizeof(portSTACK_TYPE)),  /* stack size */
                (void *)me,                             /* the 'pvParameters' parameter */
                (UBaseType_t)(priority + tskIDLE_PRIORITY),  /* FreeRTOS priority */
                (StackType_t *)stack,                   /* stack storage */
                me);                                    /* task buffer */

    elab_assert(thread != (TaskHandle_t)0);             /* must be created */

    return (osThreadId_t)thread;
}
#endif

/* ----------------------------- end of file -------------------------------- */
