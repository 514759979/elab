/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef __ELAB_EXPORT_H__
#define __ELAB_EXPORT_H__

/* include ------------------------------------------------------------------ */
#include <stdint.h>
#include "elab_def.h"

/* public define ------------------------------------------------------------ */
#define EXPORT_ID_INIT                  (0xa5a5a5a5)
#define EXPORT_ID_POLL                  (0xbeefbeef)

/* public define ------------------------------------------------------------ */
enum elab_export_level
{
    EXPORT_BSP = 0,
    EXPORT_IO_DRIVER,
    EXPORT_COMPONENT,
    EXPORT_DEVICE,
    EXPORT_APP,
    EXPORT_TEST,

    EXPORT_MAX,
};

/* public typedef ----------------------------------------------------------- */
typedef struct elab_export_data
{
    uint32_t timeout_ms;
} elab_export_poll_data_t;

typedef struct elab_export
{
    uint32_t magic_head;
    void *data;
    uint32_t period_ms;
    uint8_t level;
    void (* func)(void);
    uint32_t magic_tail;
} elab_export_t;

/* private function --------------------------------------------------------- */
void elab_unit_test(void);
void elab_run(void);

/* public export ------------------------------------------------------------ */
#define INIT_EXPORT(_func, _level)                                             \
    ELAB_USED const elab_export_t init_##_func ELAB_SECTION("elab_export") =   \
    {                                                                          \
        .func = &_func,                                                        \
        .level = _level,                                                       \
        .magic_head = EXPORT_ID_INIT,                                          \
        .magic_tail = EXPORT_ID_INIT,                                          \
    }

#define POLL_EXPORT(_func, _period_ms)                                         \
    static elab_export_poll_data_t poll_##_data =                              \
    {                                                                          \
        .timeout_ms = 0,                                                       \
    };                                                                         \
    ELAB_USED const elab_export_t poll_##_func ELAB_SECTION("expoll") =        \
    {                                                                          \
        .func = &_func,                                                        \
        .data = (void *)&poll_##_data,                                         \
        .level = EXPORT_MAX,                                                   \
        .period_ms = (uint32_t)(_period_ms),                                   \
        .magic_head = EXPORT_ID_POLL,                                          \
        .magic_tail = EXPORT_ID_POLL,                                          \
    }

/* public function ---------------------------------------------------------- */
#define INIT_BSP_EXPORT(func)               INIT_EXPORT(func, EXPORT_BSP)
#define INIT_IO_DRIVER_EXPORT(func)         INIT_EXPORT(func, EXPORT_IO_DRIVER)
#define INIT_COMPONENT_EXPORT(func)         INIT_EXPORT(func, EXPORT_COMPONENT)
#define INIT_DEV_EXPORT(func)               INIT_EXPORT(func, EXPORT_DEVICE)
#define INIT_APP_EXPORT(func)               INIT_EXPORT(func, EXPORT_APP)
#define INIT_EXPORT_TEST(func)              INIT_EXPORT(func, EXPORT_TEST)

#endif /* __ELAB_EXPORT_H__ */

/* ----------------------------- end of file -------------------------------- */
