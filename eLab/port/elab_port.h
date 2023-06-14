/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef __ELAB_PORT_H__
#define __ELAB_PORT_H__

#include "elab_config.h"

#if (ELAB_RTOS_CMSIS_OS_EN != 0)

/* public defines ----------------------------------------------------------- */
#if defined(__linux__)

#include "../RTOS/cmsis_os.h"
#define elab_thread_t                   osThreadId_t

#elif defined(__free_rtos__) || defined(__win32__)

#include "FreeRTOS.h"  /* FreeRTOS master include file, see NOTE4 */
#include "task.h"      /* FreeRTOS task  management */

#define elab_thread_t                   StaticTask_t

#endif

/* public typedef ----------------------------------------------------------- */

#endif
#endif /* __ELAB_PORT_H__ */

/* ----------------------------- end of file -------------------------------- */
