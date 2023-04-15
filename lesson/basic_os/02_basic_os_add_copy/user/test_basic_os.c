/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "basic_os.h"

/* private functions -------------------------------------------------------- */
static void _func_basic_os(void *parameter)
{
    while (1)
    {
        eos_delay_ms(100);
    }
}

task_export(poll, _func_basic_os, 1, NULL);
/* ----------------------------- end of file -------------------------------- */
