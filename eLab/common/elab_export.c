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

/* private variables -------------------------------------------------------- */
INIT_COMPONENT_EXPORT(module_null_init);
POLL_EXPORT(module_null_init);

/* public function ---------------------------------------------------------- */
/**
  * @brief  eLab initialization exporting function.
  * @param  level       init level.
  * @retval None
  */
void elab_init(uint8_t level)
{
    elab_export_init_t *export_func_table = NULL;

    /* Get the start address of exported init table. */
    elab_export_init_t *func_block = (elab_export_init_t *)&init_module_null_init;
    export_func_table = func_block;
    while (1)
    {
        uint32_t address_last = ((uint32_t)func_block - sizeof(elab_export_init_t));
        func_block = (elab_export_init_t *)address_last;
        if (func_block->magic_head == 0xa5a5a5a5 &&
            func_block->magic_tail == 0xa5a5a5a5 &&
            func_block->level < EXPORT_MAX)
        {
            export_func_table = func_block;
        }
        else
        {
            break;
        }
    }

    /* Execute the init function in the specific level. */
    for (uint32_t i = 0; ; i ++)
    {
        if (export_func_table[i].magic_head == 0xa5a5a5a5 &&
            export_func_table[i].magic_tail == 0xa5a5a5a5 &&
            export_func_table[i].level < EXPORT_MAX)
        {
            if (export_func_table[i].level == level)
            {
                export_func_table[i].func();
            }
        }
        else
        {
            break;
        }
    }
}

/**
  * @brief  eLab unit test exporting function.
  * @retval None
  */
void elab_unit_test(void)
{
    elab_init(EXPORT_TEST);
}

/**
  * @brief  eLab polling exporting function.
  * @retval None
  */
void elab_run(void)
{
    elab_export_poll_t *export_func_table = NULL;

    while (1)
    {
        /* Get the start address of exported poll table. */
        elab_export_poll_t *func_block = (elab_export_poll_t *)&poll_module_null_init;
        export_func_table = func_block;
        while (1)
        {
            uint32_t address_last = ((uint32_t)func_block - sizeof(elab_export_poll_t));
            func_block = (elab_export_poll_t *)address_last;
            if (func_block->magic_head == 0xbeefbeef &&
                func_block->magic_tail == 0xbeefbeef)
            {
                export_func_table = func_block;
            }
            else
            {
                break;
            }
        }

        /* Execute the poll function in the specific level. */
        for (uint32_t i = 0; ; i ++)
        {
            if (export_func_table[i].magic_head == 0xbeefbeef &&
                export_func_table[i].magic_tail == 0xbeefbeef)
            {
                export_func_table[i].func();
            }
            else
            {
                break;
            }
        }
    }
}

/* private function --------------------------------------------------------- */
static void module_null_init(void)
{
    /* NULL */
}

/* ----------------------------- end of file -------------------------------- */
