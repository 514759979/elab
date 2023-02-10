/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include "elab.h"

/* public functions --------------------------------------------------------- */
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    elab_run();
}

/* ----------------------------- end of file -------------------------------- */


/* Private functions ---------------------------------------------------------*/
#include <stdio.h>

static void _export_test_bsp(void)
{
    printf("export bsp testing passed.\n");
}

static void _export_test_io_driver(void)
{
    printf("export io driver testing passed.\n");
}

static void _export_test_component(void)
{
    printf("export component testing passed.\n");
}

static void _export_test_device(void)
{
    printf("export device testing passed.\n");
}

static void _export_test_app(void)
{
    printf("export app testing passed.\n");
}

static void _export_test_poll(void)
{
    printf("export poll testing passed.\n");
}

INIT_BSP_EXPORT(_export_test_bsp);
INIT_IO_DRIVER_EXPORT(_export_test_io_driver);
INIT_COMPONENT_EXPORT(_export_test_component);
INIT_DEV_EXPORT(_export_test_device);
INIT_APP_EXPORT(_export_test_app);

POLL_EXPORT(_export_test_poll, 1000);