/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef __ELAB_PORT_H__
#define __ELAB_PORT_H__

#if (ELAB_RTOS_CMSIS_OS_EN != 0)
#include "FreeRTOS.h"  /* FreeRTOS master include file, see NOTE4 */
#include "task.h"      /* FreeRTOS task  management */


/* public defines ----------------------------------------------------------- */
#define elab_thread_t                   StaticTask_t

/* public typedef ----------------------------------------------------------- */

#endif
#endif /* __ELAB_PORT_H__ */

/* ----------------------------- end of file -------------------------------- */
