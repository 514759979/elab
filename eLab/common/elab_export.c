/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "elab_export.h"
#include "elab_common.h"

#if (ELAB_RTOS_CMSIS_OS_EN != 0)
#include "cmsis_os.h"
#endif
#if (ELAB_RTOS_BASIC_OS_EN != 0)
#include "basic_os.h"
#endif

#if (ELAB_QPC_EN != 0)
Q_DEFINE_THIS_FILE
#include "qpc.h"
#endif

#if (ELAB_RTOS_BASIC_OS_EN != 0)
#define ELAB_GLOBAL_STACK_SIZE                      (4096)
#endif

/* private function prototype ----------------------------------------------- */
static void module_null_init(void);
static void _export_func_execute(uint8_t level);
#if (ELAB_RTOS_CMSIS_OS_EN != 0 || ELAB_RTOS_BASIC_OS_EN != 0)
static void _entry_start_poll(void *para);
#endif

/* private variables -------------------------------------------------------- */
INIT_BSP_EXPORT(module_null_init);
POLL_EXPORT(module_null_init, (1000 * 60 * 60));

static const uint32_t export_id_table[EXPORT_MAX + 1] =
{
    EXPORT_ID_INIT,
    EXPORT_ID_INIT,
    EXPORT_ID_INIT,
    EXPORT_ID_INIT,
    EXPORT_ID_INIT,
#if (ELAB_RTOS_CMSIS_OS_EN != 0)
    EXPORT_ID_INIT,
#endif
#if (ELAB_QPC_EN != 0)
    EXPORT_ID_INIT,
#endif
    EXPORT_ID_INIT,
    EXPORT_ID_POLL,
};

static elab_export_t *export_init_table = NULL;
static elab_export_t *export_poll_table = NULL;

#if (ELAB_RTOS_CMSIS_OS_EN != 0)
/**
 * @brief  The thread attribute for testing.
 */
static const osThreadAttr_t thread_attr_start_poll = 
{
    .name = "ThreadStartPoll",
    .attr_bits = osThreadDetached,
    .priority = osPriorityNormal,
    .stack_size = 2048,
};
#endif

/* public function ---------------------------------------------------------- */
/**
  * @brief  eLab unit test exporting function.
  * @retval None
  */
void elab_unit_test(void)
{
    _export_func_execute(EXPORT_TEST);
}

/**
  * @brief  eLab polling exporting function.
  * @retval None
  */
void elab_run(void)
{
    /* Start polling function in metal eLab, or start the RTOS kernel in RTOS 
       eLab. */
#if (ELAB_RTOS_CMSIS_OS_EN != 0 || ELAB_RTOS_BASIC_OS_EN != 0)
    osKernelInitialize();
#endif
#if (ELAB_RTOS_CMSIS_OS_EN != 0)
    osThreadNew(_entry_start_poll, NULL, &thread_attr_start_poll);
#endif
#if (ELAB_RTOS_CMSIS_OS_EN != 0 || ELAB_RTOS_BASIC_OS_EN != 0)
    osKernelStart();
#else
    /* Initialize all module in eLab. */
    for (uint8_t level = EXPORT_BSP; level <= EXPORT_APP; level ++)
    {
        _export_func_execute(level);
    }

    /* Start polling function in metal eLab. */
    while (1)
    {
        _export_func_execute(EXPORT_MAX);
    }
#endif
}

/* private function --------------------------------------------------------- */
/**
  * @brief  Get the export table.
  */
static elab_export_t * _get_export_table(uint8_t level)
{
    elab_export_t *func_block = 
                    level < EXPORT_MAX ?
                    ((elab_export_t *)&init_module_null_init) :
                    ((elab_export_t *)&poll_module_null_init);
    
    while (1)
    {
        elab_pointer_t address_last = ((elab_pointer_t)func_block - sizeof(elab_export_t));
        elab_export_t *table = (elab_export_t *)address_last;
        if (table->magic_head != export_id_table[level] ||
            table->magic_tail != export_id_table[level])
        {
            break;
        }
        func_block = table;
    }

    return func_block;
}

/**
  * @brief  eLab exporting function executing.
  * @param  level export level.
  * @retval None
  */
static void _export_func_execute(uint8_t level)
{
    uint32_t export_id = export_id_table[level];

    /* Get the start address of exported poll table. */
    if (export_init_table == NULL)
    {
        export_init_table = _get_export_table(EXPORT_BSP);
    }
    if (export_poll_table == NULL)
    {
        export_poll_table = _get_export_table(EXPORT_MAX);
    }

    /* Execute the poll function in the specific level. */
    elab_export_t *export_table = level < EXPORT_MAX ?
                                    export_init_table : export_poll_table;
    for (uint32_t i = 0; ; i ++)
    {
        if (export_table[i].magic_head == export_id &&
            export_table[i].magic_tail == export_id)
        {
            if (export_table[i].level == level && level <= EXPORT_APP)
            {
                ((void (*)(void))export_table[i].func)();
            }
#if (ELAB_RTOS_CMSIS_OS_EN != 0)
            else if (export_table[i].level == level && level == EXPORT_THREAD)
            {
                elab_thread_init((elab_thread_t *)export_table[i].object,
                                    (void (*)(void *))export_table[i].func,
                                    export_table[i].name, export_table[i].data,
                                    export_table[i].stack,
                                    export_table[i].stack_size,
                                    export_table[i].priority);
            }
#endif
#if (ELAB_QPC_EN != 0)
            else if (export_table[i].level == level && level == EXPORT_HSM)
            {
                QActive_ctor((QActive *)export_table[i].object,
                                Q_STATE_CAST(export_table[i].func));
                QACTIVE_START((QActive *)export_table[i].object,
                                export_table[i].priority,
                                export_table[i].data, export_table[i].queue_size,
                                export_table[i].stack, export_table[i].stack_size,
                                (QEvt *)0);
            }
#endif
            else if (export_table[i].level == level && level == EXPORT_MAX)
            {
                elab_export_poll_data_t *data = export_table[i].data;
                while (elab_time_ms() >= data->timeout_ms)
                {
                    data->timeout_ms += export_table[i].period_ms;
                    ((void (*)(void))export_table[i].func)();
                }
            }
        }
        else
        {
            break;
        }
    }
}

/**
  * @brief  eLab startup and poll function.
  * @retval None
  */
static void _entry_start_poll(void *para)
{
    /* Initialize all module in eLab. */
    for (uint8_t level = EXPORT_BSP; level <= EXPORT_APP; level ++)
    {
        _export_func_execute(level);
    }
#if (ELAB_RTOS_CMSIS_OS_EN != 0)
    _export_func_execute(EXPORT_THREAD);
#endif
#if (ELAB_QPC_EN != 0)
    _export_func_execute(EXPORT_HSM);
#endif

    /* Start polling function in metal eLab. */
    while (1)
    {
        _export_func_execute(EXPORT_MAX);
#if (ELAB_RTOS_CMSIS_OS_EN != 0 || ELAB_RTOS_BASIC_OS_EN != 0)
        osDelay(10);
#endif
    }
}

#if (ELAB_RTOS_BASIC_OS_EN != 0)
bos_task_export(poll, _entry_start_poll, 1, NULL);
#endif

/**
  * @brief  eLab null exporting function.
  * @retval None
  */
static void module_null_init(void)
{
    /* NULL */
}

/* ----------------------------- end of file -------------------------------- */
