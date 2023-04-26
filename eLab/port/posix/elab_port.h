/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef __ELAB_PORT_H__
#define __ELAB_PORT_H__

#include "elab_config.h"

#if (ELAB_RTOS_CMSIS_OS_EN != 0)
#include "cmsis_os.h"

/* public defines ----------------------------------------------------------- */
#define elab_thread_t                   osThreadId_t

/* public typedef ----------------------------------------------------------- */

#endif
#endif /* __ELAB_PORT_H__ */

/* ----------------------------- end of file -------------------------------- */
