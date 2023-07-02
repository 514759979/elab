/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include "elab_def.h"
#include "elab_export.h"
#include "elab_common.h"
#include "elab_assert.h"

ELAB_TAG("eLabExport");

#if (ELAB_RTOS_CMSIS_OS_EN != 0)
#include "../RTOS/cmsis_os.h"
#endif
#if (ELAB_RTOS_BASIC_OS_EN != 0)
#include "basic_os.h"
#endif

#if (ELAB_QPC_EN != 0)
#include "../3rd/qpc/include/qpc.h"
Q_DEFINE_THIS_FILE
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if (ELAB_RTOS_BASIC_OS_EN != 0)
#define ELAB_GLOBAL_STACK_SIZE                      (4096)
#endif

/* private function prototype ----------------------------------------------- */
static void module_null_init(void);
static void _export_func_execute(int8_t level);
#if (ELAB_RTOS_CMSIS_OS_EN != 0 || ELAB_RTOS_BASIC_OS_EN != 0)
static void _entry_start_poll(void *para);
#endif

/* private variables -------------------------------------------------------- */
INIT_EXPORT(module_null_init, 0);
POLL_EXPORT(module_null_init, (1000 * 60 * 60));

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

#if (ELAB_RTOS_CMSIS_OS_EN == 0 && ELAB_RTOS_BASIC_OS_EN == 0)
static bool app_exit = false;
#endif
#if (ELAB_RTOS_CMSIS_OS_EN != 0 || ELAB_RTOS_BASIC_OS_EN != 0)
static bool app_exit_end = false;
#endif

#if defined(__linux__)
static void signal_handler(int sig)
{
    printf("Elab Signal: %d.\n", sig);

    elab_exit();
    system("stty echo");
    printf("\033[0;0m\n");

    app_exit_end = true;
    exit(-1);
}
#endif

/**
  * @brief  eLab polling exporting function.
  * @retval None
  */
void elab_run(void)
{
#if defined(__linux__)
    signal(SIGINT, signal_handler);                 /* Ctrl + C*/
    signal(SIGTERM, signal_handler);                /* kill pid */
    signal(SIGABRT, signal_handler);
    signal(SIGKILL, signal_handler);                /* kill -9 pid */
    signal(SIGHUP, signal_handler);
    signal(SIGSEGV, signal_handler);
#endif

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
    
    if (!app_exit_end)
    {
        elab_exit();
    }
#else
    /* Initialize all module in eLab. */
    for (uint8_t level = EXPORT_LEVEL_HW_INDEPNEDENT;
            level <= EXPORT_APP; level ++)
    {
        _export_func_execute(level);
    }

    /* Start polling function in metal eLab. */
    while (!app_exit)
    {
        _export_func_execute(EXPORT_MAX);
    }
#endif
}

void elab_exit(void)
{
    /* Initialize all module in eLab. */
    for (int32_t level = -EXPORT_APP; level <= -EXPORT_LEVEL_HW_INDEPNEDENT; level ++)
    {
        _export_func_execute(level - 1);
    }
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
    elab_pointer_t address_last;
    uint32_t export_id = level == EXPORT_MAX ? EXPORT_ID_POLL : EXPORT_ID_INIT;
    
    while (1)
    {
        address_last = ((elab_pointer_t)func_block - sizeof(elab_export_t));
        elab_export_t *table = (elab_export_t *)address_last;
        if (table->magic_head != export_id ||
            table->magic_tail != export_id)
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
static void _export_func_execute(int8_t level)
{
    bool is_init = level >= 0 ? true : false;
    level = level < 0 ? (-level - 1) : level;
    elab_assert(level <= EXPORT_MAX);
    uint32_t export_id = level == EXPORT_MAX ? EXPORT_ID_POLL : EXPORT_ID_INIT;

    /* Get the start address of exported poll table. */
    if (export_init_table == NULL)
    {
        export_init_table = _get_export_table(EXPORT_LEVEL_HW_INDEPNEDENT);
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
                if (is_init && export_table[i].type == EXPORT_TYPE_INIT)
                {
                    printf("Export init %s.\n", export_table[i].name);
                    ((void (*)(void))export_table[i].func)();
                }
                if (!is_init && export_table[i].type == EXPORT_TYPE_EXIT)
                {
                    printf("Export exit %s.\n", export_table[i].name);
                    ((void (*)(void))export_table[i].func)();
                }
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

#if (ELAB_RTOS_CMSIS_OS_EN != 0 || ELAB_RTOS_BASIC_OS_EN != 0)
/**
  * @brief  eLab startup and poll function.
  * @retval None
  */
static void _entry_start_poll(void *para)
{
    /* Initialize all module in eLab. */
    for (uint8_t level = EXPORT_LEVEL_HW_INDEPNEDENT;
            level < EXPORT_LEVEL_MAX; level ++)
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
#endif

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

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
