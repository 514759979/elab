/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include <stdlib.h>
#include <stdint.h>
#include "elab_export.h"
#include "elab_common.h"

/* private function prototype ----------------------------------------------- */
static void module_null_init(void);
static void _export_func_execute(uint8_t level);

/* private variables -------------------------------------------------------- */
INIT_COMPONENT_EXPORT(module_null_init);
POLL_EXPORT(module_null_init, (1000 * 60 * 60));

static const uint32_t export_id_table[EXPORT_MAX + 1] =
{
    EXPORT_ID_INIT,
    EXPORT_ID_INIT,
    EXPORT_ID_INIT,
    EXPORT_ID_INIT,
    EXPORT_ID_INIT,
    EXPORT_ID_INIT,
    EXPORT_ID_POLL,
};

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
    /* Initialize all module in eLab. */
    for (uint8_t level = EXPORT_BSP; level <= EXPORT_APP; level ++)
    {
        _export_func_execute(level);
    }

    /* Start polling function in metal eLab, or start the RTOS kernel in RTOS 
       eLab. */
    while (1)
    {
        _export_func_execute(EXPORT_MAX);
    }
}

/* private function --------------------------------------------------------- */
/**
  * @brief  eLab exporting function executing.
  * @retval None
  */
static void _export_func_execute(uint8_t level)
{
    uint32_t export_id = export_id_table[level];
    elab_export_t *func_block = 
                    level < EXPORT_MAX ?
                    ((elab_export_t *)&init_module_null_init) :
                    ((elab_export_t *)&poll_module_null_init);

    /* Get the start address of exported poll table. */
    elab_export_t *export_table = func_block;
    while (1)
    {
        uint32_t address_last = ((uint32_t)func_block - sizeof(elab_export_t));
        func_block = (elab_export_t *)address_last;
        if (func_block->magic_head == export_id &&
            func_block->magic_tail == export_id)
        {
            export_table = func_block;
        }
        else
        {
            break;
        }
    }

    /* Execute the poll function in the specific level. */
    for (uint32_t i = 0; ; i ++)
    {
        if (export_table[i].magic_head == export_id &&
            export_table[i].magic_tail == export_id)
        {
            if (export_table[i].level == level && level < EXPORT_MAX)
            {
                export_table[i].func();
            }
            else if (export_table[i].level == level && level == EXPORT_MAX)
            {
                elab_export_poll_data_t *data = export_table[i].data;
                while (elab_time_ms() >= data->timeout_ms)
                {
                    data->timeout_ms += export_table[i].period_ms;
                    export_table[i].func();
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
  * @brief  eLab null exporting function.
  * @retval None
  */
static void module_null_init(void)
{
    /* NULL */
}

/* ----------------------------- end of file -------------------------------- */
