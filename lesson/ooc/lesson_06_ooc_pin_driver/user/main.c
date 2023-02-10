/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include "bsp.h"
#include "elab_export.h"

/* public functions --------------------------------------------------------- */
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    bsp_init();
    
    elab_init(EXPORT_BSP);
    elab_init(EXPORT_IO_DRIVER);

    elab_run();
}

/* ----------------------------- end of file -------------------------------- */
