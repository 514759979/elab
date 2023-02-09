/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef __ELAB_EXPORT_H__
#define __ELAB_EXPORT_H__

/* include ------------------------------------------------------------------ */
#include <stdint.h>

/* public define ------------------------------------------------------------ */
#ifndef EXPORT_SECTION
    #if defined(__CC_ARM) || defined(__CLANG_ARM) || defined(__GNUC__)
        #define EXPORT_SECTION(x)               __attribute__((section(x)))
    #elif defined (__IAR_SYSTEMS_ICC__)
        #define EXPORT_SECTION(x)               @ x
    #else
        #error The compiler is not supported!
    #endif
#endif

#ifndef EXPORT_USED
    #if defined(__CC_ARM) || defined(__CLANG_ARM) || defined(__GNUC__)
        #define EXPORT_USED                     __attribute__((used))
    #elif defined (__IAR_SYSTEMS_ICC__)
        #define EXPORT_USED                     __root
    #else
        #error The compiler is not supported!
    #endif
#endif

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
typedef struct elab_export_init
{
    uint32_t magic_head;
    uint8_t level;
    void (* func)(void);
    uint32_t magic_tail;
} elab_export_init_t;

typedef struct elab_export_poll_func
{
    uint32_t magic_head;
    void (* func)(void);
    uint32_t magic_tail;
} elab_export_poll_t;

/* private function --------------------------------------------------------- */
void elab_init(uint8_t level);
void elab_unit_test(void);
void elab_run(void);

/* public export ------------------------------------------------------------ */
#define INIT_EXPORT(_func, _level)                                                      \
    EXPORT_USED const elab_export_init_t init_##_func EXPORT_SECTION("elab_export") =   \
    {                                                                                   \
        .func = &_func,                                                                 \
        .level = _level,                                                                \
        .magic_head = 0xa5a5a5a5,                                                       \
        .magic_tail = 0xa5a5a5a5,                                                       \
    }

/* TODO Add the polling period setting function. */
#define POLL_EXPORT(_func)                                                              \
    EXPORT_USED const elab_export_poll_t poll_##_func EXPORT_SECTION("expoll") =        \
    {                                                                                   \
        .func = &_func,                                                                 \
        .magic_head = 0xbeefbeef,                                                       \
        .magic_tail = 0xbeefbeef,                                                       \
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
