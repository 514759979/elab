/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "elab.h"

/* private state function prototypes -----------------------------------------*/
static void _entry(void *para);

THREAD_EXPORT(test, _entry, osPriorityNormal, NULL, 1024);

/* private state function ----------------------------------------------------*/
static void _entry(void *para)
{
    while (1)
    {
        printf("Thread exporting testing passed.\n");
        osDelay(1000);
    }
}

/* ----------------------------- end of file -------------------------------- */
