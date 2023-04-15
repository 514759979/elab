
/*
 * EventOS Basic
 * Copyright (c) 2021, EventOS Team, <event-os@outlook.com>
 *
 * SPDX-License-Identifier: MIT
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the 'Software'), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS 
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.event-os.cn
 * https://github.com/event-os/eventos-basic
 * https://gitee.com/event-os/eventos-basic
 * 
 * Change Logs:
 * Date           Author        Notes
 * 2022-03-21     DogMing       V0.0.1
 */

#ifndef EVENTOS_H_
#define EVENTOS_H_

/* include ------------------------------------------------------------------ */
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* public define ------------------------------------------------------------ */
#define EXPORT_ID_TASK                  (0xa5a5a5a5)
#define EXPORT_ID_TIMER                 (0xbeefbeef)

/* Config ------------------------------------------------------------------- */
// 支持的最大的线程数
#define EOS_MAX_TASKS                           32

// 支持的最大优先级数
#define EOS_MAX_PRIORITY                        8

// 时钟滴答的毫秒数
#define EOS_TICK_MS                             1

// 是否使用断言
#define EOS_USE_ASSERT                          1

// 是否统计堆栈使用率
#define EOS_USE_STACK_USAGE                     0

// 是否统计CPU使用率
#define EOS_USE_CPU_USAGE                       1

#if (EOS_MAX_TASKS > 32)
#error The number of tasks can NOT be larger than 32 !
#endif

/* Data structure ----------------------------------------------------------- */
enum
{
    EOS_OK                          = 0,
    EOS_NOT_FOUND                   = -1,

};

typedef void (* eos_func_t)(void *parameter);

typedef struct eos_task_rom
{
    uint32_t magic_head;
    eos_func_t func;
    uint32_t priority;
    const char *name;
    void *parameter;
    void *data;
    bool oneshoot;
    uint32_t magic_tail;
} eos_task_rom_t;

typedef struct eos_timer_rom
{
    uint32_t magic_head;
    eos_func_t func;
    uint32_t period;
    const char *name;
    void *parameter;
    void *data;
    bool oneshoot;
    uint32_t magic_tail;
} eos_timer_rom_t;

/* Task related. */
typedef struct eos_task
{
    uint32_t *sp;
    uint32_t timeout;
    void *stack;
    uint32_t stack_size             : 16;
    uint32_t state                  : 4;
    uint32_t state_bkp              : 4;
    uint32_t task_id                : 8;
} eos_task_t;

/* Timer related. */
typedef struct eos_timer
{
    uint32_t time;
    uint32_t id                     : 10;
    uint32_t domain                 : 8;
    uint32_t running                : 1;
} eos_timer_t;

/* Compiler Related Definitions */
#if defined(__CC_ARM) || defined(__CLANG_ARM) /* ARM Compiler */
    #include <stdarg.h>
    #define BOS_SECTION(x)              __attribute__((section(x)))
    #define BOS_USED                    __attribute__((used))

#elif defined (__IAR_SYSTEMS_ICC__)           /* for IAR Compiler */
    #include <stdarg.h>
    #define BOS_SECTION(x)              @ x
    #define BOS_USED                    __root

#elif defined (__GNUC__)                      /* GNU GCC Compiler */
    #include <stdarg.h>
    #define BOS_SECTION(x)              __attribute__((section(x)))
    #define BOS_USED                    __attribute__((used))
#else
    #error The compiler is not supported!
#endif

/* 任务相关 ------------------------------------------------------------------ */
// 初始化，建议在main函数中调用。
void eos_init(void *stack, uint32_t size);
// 启动系统
void eos_run(void);
// 系统当前时间
uint32_t eos_time(void);
// 系统滴答，建议在SysTick中断里调用，也可在普通定时器中断中调用。
void eos_tick(void);
// 任务内延时，任务函数中调用，不允许在定时器的回调函数调用，不允许在空闲回调函数中调用。
void eos_delay_ms(uint32_t time_ms);
// 退出任务，任务函数中调用。
void eos_task_exit(void);
// 任务切换
void eos_task_yield(void);

#define task_export(_name, _func, _priority, para)                                \
    static eos_task_t task_##_name##_data;                                      \
    BOS_USED const eos_task_rom_t task_##_name BOS_SECTION("task_export") =     \
    {                                                                          \
        .name = #_name,                                                         \
        .func = _func,                                                 \
        .priority = _priority,                                                  \
        .parameter = para,                                                     \
        .data = &task_##_name##_data,                                           \
        .magic_head = EXPORT_ID_TASK,                                          \
        .magic_tail = EXPORT_ID_TASK,                                          \
    }

#define timer_export(_name, _func, _period, _oneshoot, _para)                       \
    static eos_timer_t timer_##_name##_data;                                     \
    BOS_USED const eos_timer_rom_t tim_##_name BOS_SECTION("timer_export") =   \
    {                                                                          \
        .name = (const char *)#_name,                                            \
        .func = _func,                                                \
        .oneshoot = _oneshoot, \
        .parameter = _para,                                                     \
        .data = (void *)&timer_##_name##_data,                                           \
        .magic_head = EXPORT_ID_TIMER,                                         \
        .magic_tail = EXPORT_ID_TIMER,                                         \
    }

/* 软定时器 ------------------------------------------------------------------ */
// 获取定时器的ID
int16_t eos_timer_get_id(const char *name);
// 启动软定时器
void eos_timer_start(uint16_t timer_id);
// 暂停软定时器，允许在中断中调用。
void eos_timer_pause(uint16_t timer_id);
// 继续软定时器，允许在中断中调用。
void eos_timer_continue(uint16_t timer_id);
// 重启软定时器的定时，允许在中断中调用。
void eos_timer_reset(uint16_t timer_id);

/* port --------------------------------------------------------------------- */
void eos_port_assert(uint32_t error_id);

/* hook --------------------------------------------------------------------- */
// 空闲回调函数
void eos_hook_idle(void);

// 启动EventOS Basic的时候，所调用的回调函数
void eos_hook_start(void);

#ifdef __cplusplus
}
#endif

#endif
