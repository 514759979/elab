/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef __ELAB_EXPORT_H__
#define __ELAB_EXPORT_H__

/* include ------------------------------------------------------------------ */
#include <stdint.h>
#include "elab_def.h"
#if (ELAB_RTOS_CMSIS_OS_EN != 0)
#include "../port/elab_port.h"
#endif

#if (ELAB_QPC_EN != 0)
#include "../3rd/qpc/include/qpc.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* public define ------------------------------------------------------------ */
#define EXPORT_ID_INIT                  (0xa5a5a5a5)
#define EXPORT_ID_POLL                  (0xbeefbeef)

/* public define ------------------------------------------------------------ */
enum elab_export_type
{
    EXPORT_TYPE_INIT = 0,
    EXPORT_TYPE_EXIT,
};

/* public typedef ----------------------------------------------------------- */
typedef struct elab_export_poll_data
{
    uint32_t timeout_ms;
} elab_export_poll_data_t;

typedef struct elab_export
{
    uint32_t magic_head;
    const char *name;
    void *data;
    void *stack;
    void *object;
    void *func;
    uint16_t stack_size;
    uint16_t queue_size;
    uint16_t priority;
    uint8_t level;
    uint8_t type;
    uint32_t period_ms;
    uint32_t magic_tail;
} elab_export_t;

/* private function --------------------------------------------------------- */
void elab_unit_test(void);
void elab_run(void);
void elab_exit(void);

/* public export ------------------------------------------------------------ */
/**
  * @brief  Initialization function exporting macro.
  * @param  _func   The polling function.
  * @param  _level  The export level. See enum elab_export_level.
  * @retval None.
  */
#define INIT_EXPORT(_func, _level)                                             \
    ELAB_USED const elab_export_t init_##_func ELAB_SECTION("elab_export") =   \
    {                                                                          \
        .name = #_func,                                                        \
        .func = (void *)&_func,                                                \
        .level = _level,                                                       \
        .type = (uint8_t)EXPORT_TYPE_INIT,                                     \
        .magic_head = EXPORT_ID_INIT,                                          \
        .magic_tail = EXPORT_ID_INIT,                                          \
    }

/**
  * @brief  Exiting function exporting macro.
  * @param  _func   The polling function.
  * @param  _level  The export level. See enum elab_export_level.
  * @retval None.
  */
#define EXIT_EXPORT(_func, _level)                                             \
    ELAB_USED const elab_export_t exit_##_func ELAB_SECTION("elab_export") =   \
    {                                                                          \
        .name = #_func,                                                        \
        .func = (void *)&_func,                                                \
        .level = _level,                                                       \
        .type = (uint8_t)EXPORT_TYPE_EXIT,                                     \
        .magic_head = EXPORT_ID_INIT,                                          \
        .magic_tail = EXPORT_ID_INIT,                                          \
    }

#if (ELAB_QPC_EN != 0)
#if (ELAB_RTOS_CMSIS_OS_EN != 0)
/**
  * @brief  State machine exporting macro.
  * @param  _name       name of the state machine.
  * @param  me          The state machine object.
  * @param  _state_init The initial state of the state machine.
  * @param  _priority   The priority of the state machine.
  * @param  _queue_size The queue size of the state machine.
  * @param  _stack_size The stack size of the state machine's internal thread.
  * @retval None.
  */
#define HSM_EXPORT(_name, me, _state_init, _priority, _queue_size, _stack_size)\
    static QEvt const * sm_##_name##_queue[_queue_size];                       \
    static uint8_t sm_##_name##_stack[_stack_size];                            \
    ELAB_USED const elab_export_t sm_##_name ELAB_SECTION("elab_export") =     \
    {                                                                          \
        .name = (const char *)#_name,                                          \
        .object = (void *)me,                                                  \
        .func = (void *)(&_state_init),                                        \
        .data = (void *)sm_##_name##_queue,                                    \
        .queue_size = _queue_size,                                             \
        .stack = (void *)sm_##_name##_stack,                                   \
        .stack_size = _stack_size,                                             \
        .priority = _priority,                                                 \
        .level = EXPORT_HSM,                                                   \
        .magic_head = EXPORT_ID_INIT,                                          \
        .magic_tail = EXPORT_ID_INIT,                                          \
    }
#else

/**
  * @brief  State machine exporting macro.
  * @param  _name       name of the state machine.
  * @param  me          The state machine object.
  * @param  _state_init The initial state of the state machine.
  * @param  _priority   The priority of the state machine.
  * @param  _queue_size The queue size of the state machine.
  * @retval None.
  */
#define HSM_EXPORT(_name, me, _state_init, _priority, _queue_size)             \
    static QEvt const * sm_##_name##_queue[_queue_size];                       \
    ELAB_USED const elab_export_t sm_##_name ELAB_SECTION("elab_export") =     \
    {                                                                          \
        .name = (const char *)#_name,                                          \
        .object = (void *)me,                                                  \
        .func = (void *)(&_state_init),                                        \
        .data = (void *)sm_##_name##_queue,                                    \
        .queue_size = _queue_size,                                             \
        .priority = _priority,                                                 \
        .level = EXPORT_HSM,                                                   \
        .magic_head = EXPORT_ID_INIT,                                          \
        .magic_tail = EXPORT_ID_INIT,                                          \
    }
#endif
#else
#define HSM_EXPORT(_name, me, _state_init, _priority, _queue_size, _stack_size)
#endif

#if (ELAB_RTOS_CMSIS_OS_EN != 0)
/**
  * @brief  Thread exporting macro.
  * @param  _name       name of the state machine.
  * @param  _entry      The thread entry function.
  * @param  _priority   The priority of the state machine.
  * @param  _data       user data.
  * @param  _stack_size The stack size of the state machine.
  * @retval None.
  */
#define THREAD_EXPORT(_name, _entry, _priority, _data, _stack_size)            \
    static elab_thread_t thread_##_name;                                       \
    static uint8_t thread_##_name##_stack[_stack_size];                        \
    ELAB_USED const elab_export_t thread##_name ELAB_SECTION("elab_export") =  \
    {                                                                          \
        .func = (void *)&_entry,                                               \
        .name = #_name,                                                        \
        .data = (void *)_data,                                                 \
        .object = (void *)&thread_##_name,                                     \
        .stack = (void *)thread_##_name##_stack,                               \
        .stack_size = _stack_size,                                             \
        .priority = _priority,                                                 \
        .level = EXPORT_THREAD,                                                \
        .magic_head = EXPORT_ID_INIT,                                          \
        .magic_tail = EXPORT_ID_INIT,                                          \
    }
#else
#define THREAD_EXPORT(_name, _entry, _priority, _data, _stack_size)
#endif

/**
  * @brief  Poll function exporting macro.
  * @param  _func       The polling function.
  * @param  _period_ms  The polling period in ms.
  * @retval None.
  */
#define POLL_EXPORT(_func, _period_ms)                                         \
    static elab_export_poll_data_t poll_##_data =                              \
    {                                                                          \
        .timeout_ms = 0,                                                       \
    };                                                                         \
    ELAB_USED const elab_export_t poll_##_func ELAB_SECTION("expoll") =        \
    {                                                                          \
        .name = "poll",                                                        \
        .func = &_func,                                                        \
        .data = (void *)&poll_##_data,                                         \
        .level = EXPORT_MAX,                                                   \
        .period_ms = (uint32_t)(_period_ms),                                   \
        .magic_head = EXPORT_ID_POLL,                                          \
        .magic_tail = EXPORT_ID_POLL,                                          \
    }

#ifdef __cplusplus
}
#endif

#endif /* __ELAB_EXPORT_H__ */

/* ----------------------------- end of file -------------------------------- */
