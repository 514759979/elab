/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ELAB_CONFIG_H
#define ELAB_CONFIG_H

/* public config ------------------------------------------------------------ */
/* CMSIS OS related -------------------------------------- */
#define ELAB_RTOS_CMSIS_OS_EN                   (0)
#define ELAB_RTOS_TICK_MS                       (1)

/* BASIC OS related -------------------------------------- */
#define ELAB_RTOS_BASIC_OS_EN                   (1)

/* EVENT OS related -------------------------------------- */
#define ELAB_RTOS_EVENT_OS_EN                   (1)

/* QPC related ------------------------------------------- */
#define ELAB_QPC_EN                             (0)
#define ELAB_EVENT_DATA_SIZE                    (128)
#define ELAB_EVENT_POOL_SIZE                    (64)

#if (ELAB_RTOS_CMSIS_OS_EN != 0 && ELAB_RTOS_BASIC_OS_EN != 0)
    #error "CMSIS OS and BasicOS can NOT be existent in the same MCU."
#endif

#endif /* ELAB_CONFIG_H */

/* ----------------------------- end of file -------------------------------- */
