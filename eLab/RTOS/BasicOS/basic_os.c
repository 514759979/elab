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
 * 2021-11-23     GouGe         V0.1.0
 */

/* include ------------------------------------------------------------------ */
#include "basic_os.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* assert ------------------------------------------------------------------- */
#if (EOS_USE_ASSERT != 0)
#define EOS_ASSERT(test_)                                                      \
    do {                                                                       \
        if (!(test_))                                                          \
        {                                                                      \
            eos_critical_enter();                                              \
            eos_port_assert(__LINE__);                                         \
        }                                                                      \
    } while (0)
#else
#define EOS_ASSERT(test_)               ((void)0)
#endif


static void _entry_idle(void *parameter);
static void _cb_timer_tick(void *para);

task_export(task_idle, _entry_idle, 1, NULL);
timer_export(basic_timer, _cb_timer_tick, 1, false, NULL);

/* eos task ----------------------------------------------------------------- */
eos_task_t *volatile eos_current;
eos_task_t *volatile eos_next;

enum
{
    EosTaskState_Ready = 0,
    EosTaskState_Running,
    EosTaskState_Blocked,
    EosTaskState_Suspended,
    EosTaskState_Stop,

    EosTaskState_Max,
};

typedef struct eos_tag
{
    eos_task_rom_t *task_table;
    eos_timer_rom_t *timer_table;
    uint16_t task_count;
    uint16_t timer_count;
    void *stack;
    uint16_t stack_size;

    uint32_t time_idle_backup;
    uint32_t time;
    uint32_t time_out_min;
    uint32_t time_offset;
    uint32_t cpu_usage_count;
} eos_t;

eos_t eos;

/* private variables -------------------------------------------------------- */
static uint8_t memory_target[32];
static uint8_t memory_source[32];
uint32_t addr_target = 0;
uint32_t addr_source = 0;
uint32_t copy_size = 16;

/* macro -------------------------------------------------------------------- */
#define EOS_MS_NUM_30DAY                (2592000000U)
#define EOS_MS_NUM_15DAY                (1296000000U)

/* private function --------------------------------------------------------- */
static void eos_sheduler(void);
static void eos_critical_enter(void);
static void eos_critical_exit(void);

static void eos_task_start(eos_task_t * const me,
                            eos_func_t func,
                            uint8_t priority,
                            void *stack_addr,
                            uint32_t stack_size,
                            void *parameter);

/* public function ---------------------------------------------------------- */
void eos_init(void *stack, uint32_t size)
{
    for (uint32_t i = 0; i < 32; i ++)
    {
        memory_source[i] = (uint8_t)i;
        memory_target[i] = 0;
    }
    addr_target = (uint32_t)&memory_target;
    addr_source = (uint32_t)&memory_source;
    
    eos_critical_enter();

    /* Set PendSV to be the lowest priority. */
    *(uint32_t volatile *)0xE000ED20 |= (0xFFU << 16U);

    /* Set the stack and its size. */
    uint32_t mod = (uint32_t)stack % 4;
    eos.stack = mod == 0 ? stack : (void *)((uint32_t)stack - mod);
    eos.stack_size = mod == 0 ? size : (size - mod);

    /* Get the task table and its counting number. */
    eos.task_table = (eos_task_rom_t *)&task_task_idle;
    eos_task_rom_t *task_temp = NULL;
    while (1)
    {
        task_temp =
            (eos_task_rom_t *)((uint32_t)eos.task_table - sizeof(eos_task_rom_t));
        if (task_temp->magic_head != EXPORT_ID_TASK ||
            task_temp->magic_tail != EXPORT_ID_TASK)
        {
            break;
        }
        
        eos.task_table = task_temp;
    }
    eos.task_count = 0;
    uint32_t task_id_high_prio = 0;
    uint8_t priority = 0;
    for (uint32_t i = 0; ; i ++)
    {
        if (eos.task_table[i].magic_head == EXPORT_ID_TASK &&
            eos.task_table[i].magic_tail == EXPORT_ID_TASK)
        {
            if (eos.task_table[i].priority > priority)
            {
                task_id_high_prio = i;
                priority = eos.task_table[i].priority;
            }
            
            // TODO Check the tasks' data is not repeated.
            eos.task_count ++;
            

        }
        else
        {
            break;
        }
    }

    /* Get the timer table and its counting number. */
    eos.timer_table = (eos_timer_rom_t *)&tim_basic_timer;
    eos_timer_rom_t *timer_temp = NULL;
    while (1)
    {
        timer_temp =
            (eos_timer_rom_t *)((uint32_t)eos.timer_table - sizeof(eos_timer_rom_t));
        if (timer_temp->magic_head != EXPORT_ID_TIMER ||
            timer_temp->magic_tail != EXPORT_ID_TIMER)
        {
            break;
        }
        
        eos.timer_table = timer_temp;
    }
    eos.timer_count = 0;
    
    for (uint32_t i = 0; ; i ++)
    {
        if (eos.timer_table[i].magic_head == EXPORT_ID_TIMER &&
            eos.timer_table[i].magic_tail == EXPORT_ID_TIMER)
        {
            EOS_ASSERT(eos.timer_table[i].period <= EOS_MS_NUM_30DAY);

            // TODO Check all timers' data is not repeated.
            eos.timer_count ++;
        }
        else
        {
            break;
        }
    }

    eos.time = 0;
    eos.time_offset = 0;
    eos.time_out_min = UINT32_MAX;
    
    /* Get the highest priority task. */
    eos_current = NULL;
    eos_next = (eos_task_t *)eos.task_table[eos.task_count - 1].data;

    /* Set the stack RAM for every task. */
    uint32_t remaining = eos.stack_size / 4 - 16 * (eos.task_count - 1);
    void *stack_current = eos.stack;
    eos_task_t *task_data = NULL;
    eos_task_rom_t *task_info = NULL;
    for (uint32_t i = 0; i < eos.task_count; i ++)
    {
        task_data = (eos_task_t *)eos.task_table[i].data;
        task_info = (eos_task_rom_t *)&eos.task_table[i];
        task_data->stack = (void *)((uint32_t)stack_current + eos.stack_size - remaining);
        task_data->stack_size = i == task_id_high_prio ? 16 : remaining;
        task_data->task_id = i;
        remaining -= task_data->stack_size;

        eos_task_start(task_data,
                        task_info->func,
                        task_info->priority,
                        task_data->stack,
                        task_data->stack_size,
                        task_info->parameter);
    }

    eos_critical_exit();
}

static void eos_task_start(eos_task_t * const me,
                            eos_func_t func,
                            uint8_t priority,
                            void *stack_addr,
                            uint32_t stack_size,
                            void *parameter)
{
    EOS_ASSERT(priority <= EOS_MAX_PRIORITY && priority != 0);
//    EOS_ASSERT(eos_current != &task_task_idle_data);
    
    /* round down the stack top to the 8-byte boundary
     * NOTE: ARM Cortex-M stack grows down from hi -> low memory
     */
    uint32_t *sp = (uint32_t *)((((uint32_t)stack_addr + stack_size) >> 3U) << 3U);

    *(-- sp) = (uint32_t)(1 << 24);            /* xPSR, Set Bit24(Thumb Mode) to 1. */
    *(-- sp) = (uint32_t)func;                 /* the entry function (PC) */
    *(-- sp) = (uint32_t)func;                 /* R14(LR) */
    *(-- sp) = (uint32_t)0x12121212u;          /* R12 */
    *(-- sp) = (uint32_t)0x03030303u;          /* R3 */
    *(-- sp) = (uint32_t)0x02020202u;          /* r2 */
    *(-- sp) = (uint32_t)0x01010101u;          /* R1 */
    *(-- sp) = (uint32_t)parameter;            /* r0 */
    /* additionally, fake registers r4-r11 */
    *(-- sp) = (uint32_t)0x11111111u;          /* r11 */
    *(-- sp) = (uint32_t)0x10101010u;          /* r10 */
    *(-- sp) = (uint32_t)0x09090909u;          /* r9 */
    *(-- sp) = (uint32_t)0x08080808u;          /* r8 */
    *(-- sp) = (uint32_t)0x07070707u;          /* r7 */
    *(-- sp) = (uint32_t)0x06060606u;          /* r6 */
    *(-- sp) = (uint32_t)0x05050505u;          /* r5 */
    *(-- sp) = (uint32_t)0x04040404u;          /* r4 */

    /* save the top of the stack in the task's attibute */
    me->sp = sp;

    eos_critical_enter();
    me->state = EosTaskState_Ready;
    me->state_bkp = EosTaskState_Ready;
    eos_critical_exit();
}

void eos_run(void)
{
    eos_hook_start();
    
    eos_sheduler();
    
    while (1)
    {
    }
}

void eos_tick(void)
{
    eos_critical_enter();
    eos.time += EOS_TICK_MS;
    eos_critical_exit();
}

void eos_delay_ms(uint32_t time_ms)
{
    if (time_ms == 0)
    {
        eos_task_yield();
        return;
    }
    
    EOS_ASSERT(time_ms <= EOS_MS_NUM_30DAY);

    /* never call eos_delay_ms in the idle task. */
    EOS_ASSERT(eos_current != &task_task_idle_data);

    eos_critical_enter();
    eos_current->timeout = eos.time + time_ms;
    eos_current->state = EosTaskState_Blocked;
    eos_critical_exit();
    
    eos_sheduler();
}

void eos_task_exit(void)
{
    eos_critical_enter();
    /* Find the task in the task table. */
    for (int32_t i = (eos.task_count - 1); i >= 0; i --)
    {
        eos_task_t *task_data = (eos_task_t *)eos.task_table[i].data;
        if (task_data == eos_current)
        {
            eos_current->state = EosTaskState_Stop;
            goto exit;
        }
    }
    EOS_ASSERT(false);

exit:
    eos_sheduler();
}

static bool eos_check_timer(bool task_idle)
{
    bool ret = false;
    eos_timer_t *timer_data = NULL;
    eos_task_t *task_data = NULL;
    
    if (eos.time_idle_backup != eos.time)
    {
        eos.time_idle_backup = eos.time;
        
        eos_critical_enter();

        /* check all the task are timeout or not. */
        for (uint32_t i = 0; i < eos.task_count; i ++)
        {
            task_data = (eos_task_t *)eos.task_table[i].data;
            if (task_data->state == EosTaskState_Blocked)
            {
                if (eos.time >= task_data->timeout)
                {
                    task_data->state = EosTaskState_Ready;
                    if (task_idle)
                    {
                        eos_critical_exit();
                        eos_sheduler();
                        eos_critical_enter();
                    }
                }
            }
        }
        
        if (eos.time >= EOS_MS_NUM_15DAY)
        {
            /* Adjust all tasks' timing. */
            for (uint32_t i = 0; i < eos.task_count; i ++)
            {
                task_data = (eos_task_t *)eos.task_table[i].data;
                if (task_data->state == EosTaskState_Blocked)
                {
                    task_data->timeout -= eos.time;
                }
            }

            /* Adjust all timers' timing. */
            for (uint32_t i = 0; i < eos.timer_count; i ++)
            {
                timer_data = (eos_timer_t *)eos.timer_table[i].data;
                if (timer_data->running != 0)
                {
                    timer_data->time -= eos.time;
                }
            }

            eos.time_out_min -= eos.time;
            eos.time_offset += eos.time;
            eos.time = 0;
        }

        /* if any timer is timeout */
        if (eos.time >= eos.time_out_min)
        {
            /* Find the time-out timers and excute the handlers. */
            for (uint32_t i = 0; i < eos.timer_count; i ++)
            {
                timer_data = (eos_timer_t *)eos.timer_table[i].data;
                if (timer_data->running != 0 && eos.time >= timer_data->time)
                {
                    eos_critical_exit();
                    eos.timer_table[i].func(eos.timer_table[i].parameter);
                    eos_critical_enter();
                    if (eos.timer_table[i].oneshoot == 0)
                    {
                        timer_data->time += eos.timer_table[i].period;
                    }
                    else
                    {
                        timer_data->running = 0;
                    }
                }
            }

            /* Recalculate the minimum timeout value. */
            uint32_t time_out_min = UINT32_MAX;
            for (uint32_t i = 0; i < eos.timer_count; i ++)
            {
                timer_data = (eos_timer_t *)eos.timer_table[i].data;
                if (timer_data->running != 0 && time_out_min >= timer_data->time)
                {
                    time_out_min = timer_data->time;
                }
            }
            eos.time_out_min = time_out_min;
            ret = true;
        }

        eos_critical_exit();
    }
    
    return ret;
}

void eos_task_yield(void)
{
    eos_task_t *task_data = NULL;

    eos_check_timer(false);
    
    eos_critical_enter();
    
    /* Find the next task in the same priority with the current task. */
    uint8_t priority_current = eos.task_table[eos_current->task_id].priority;
    for (uint32_t i = 0; i < eos.task_count; i ++)
    {
        task_data = (eos_task_t *)eos.task_table[i].data;
        if (task_data != eos_current &&
            eos.task_table[i].priority == priority_current &&
            task_data->state == EosTaskState_Suspended)
        {
            task_data->state = EosTaskState_Ready;
            eos_current->state = EosTaskState_Suspended;
            break;
        }
    }
    eos_critical_exit();
    eos_sheduler();
}

eos_task_t *eos_get_task(void)
{
    return eos_current;
}

static void eos_sheduler(void)
{
    eos_task_t *task_data = NULL;

    eos_critical_enter();
    eos_next = &task_task_idle_data;
    uint8_t priority = 0;
    for (int32_t i = (eos.task_count - 1); i >= 0; i --)
    {
        task_data = (eos_task_t *)eos.task_table[i].data;
        if (task_data->state == EosTaskState_Ready &&
            eos.task_table[i].priority > priority)
        {
            eos_next = task_data;
        }
    }
    
    if (eos_next != eos_current)
    {
        if (eos_current != NULL)
        {
            /* Calculate the data related with shared-stack. */
            if (eos_next->task_id < eos_current->task_id)
            {
                copy_size = 0;
                addr_target = (uint32_t)eos_current->sp;
                addr_source = (uint32_t)eos_current->stack;
                for (uint32_t i = eos_next->task_id; i < eos_current->task_id; i ++)
                {
                    task_data = (eos_task_t *)eos.task_table[i].data;
                    copy_size += task_data->stack_size;
                }
            }
            else
            {
                copy_size = eos_current->stack_size + (uint32_t)eos_current->sp - (uint32_t)eos_current->stack;
                addr_target = (uint32_t)eos_current->stack;
                addr_source = (uint32_t)eos_current->sp;
                for (uint32_t i = eos_current->task_id + 1; i < eos_next->task_id; i ++)
                {
                    task_data = (eos_task_t *)eos.task_table[i].data;
                    copy_size += task_data->stack_size;
                }
            }
        }
        
        /* Trig task switching. */
        *(uint32_t volatile *)0xE000ED04 = (1U << 28);
    }
    eos_critical_exit();
}

uint32_t eos_time(void)
{
    eos_critical_enter();
    uint32_t time_offset = eos.time_offset;
    eos_critical_exit();

    return (time_offset + eos.time);
}

/* Soft timer --------------------------------------------------------------- */
int16_t eos_timer_get_id(const char *name)
{
    /* Find the timer in the task table. */
    int16_t ret = EOS_NOT_FOUND;
    for (uint32_t i = 0; i < eos.timer_count; i ++)
    {
        if (strcmp(eos.timer_table[i].name, name) == 0)
        {
            ret = i;
            break;
        }
    }

    return ret;
}

void eos_timer_start(uint16_t timer_id)
{
    EOS_ASSERT(timer_id < eos.timer_count);

    eos_critical_enter();

    eos_timer_t *timer = (eos_timer_t *)eos.timer_table[timer_id].data;
    timer->running = 1;

    eos_critical_exit();
}

void eos_timer_pause(uint16_t timer_id)
{
    EOS_ASSERT(timer_id < eos.timer_count);
    eos_timer_t *timer = NULL;
    uint32_t time_out_min = UINT32_MAX;

    eos_critical_enter();

    for (uint32_t i = 0; i < eos.timer_count; i ++)
    {
        timer = (eos_timer_t *)eos.timer_table[timer_id].data;
        if (i == timer_id)
        {
            timer->running = 0;
        }
        else if (timer->running != 0 && time_out_min > timer->time)
        {
            time_out_min = timer->time;
        }
    }
    eos.time_out_min = time_out_min;

    eos_critical_exit();
}

void eos_timer_continue(uint16_t timer_id)
{
    EOS_ASSERT(timer_id < eos.timer_count);
    eos_timer_t *timer = NULL;

    eos_critical_enter();

    bool existent = false;
    for (uint32_t i = 0; i < eos.timer_count; i ++)
    {
        timer = (eos_timer_t *)eos.timer_table[timer_id].data;
        if (i == timer_id)
        {
            timer->running = 1;
            if (eos.time_out_min > timer->time)
            {
                eos.time_out_min = timer->time;
            }
            existent = true;
            break;
        }
    }
    EOS_ASSERT(existent);

    // not found
    eos_critical_exit();
}

void eos_timer_reset(uint16_t timer_id)
{
    EOS_ASSERT(timer_id < eos.timer_count);
    eos_critical_enter();

    eos_timer_t *timer = (eos_timer_t *)eos.timer_table[timer_id].data;
    timer->running = 1;
    if (eos.time_out_min > timer->time)
    {
        eos.time_out_min = timer->time;
    }

    eos_critical_exit();
}

static void _entry_idle(void *parameter)
{
    (void)parameter;
    
    while (1)
    {
        // if no timer is timeout
        if (eos_check_timer(true))
        {
            eos_hook_idle();
        }
    }
}

static void _cb_timer_tick(void *para)
{

}

uint32_t count = 0;

void func_test(void)
{
    count ++;
}

/*******************************************************************************
* NOTE:
* The inline GNU assembler does not accept mnemonics MOVS, LSRS and ADDS,
* but for Cortex-M0/M0+/M1 the mnemonics MOV, LSR and ADD always set the
* condition flags in the PSR.
*******************************************************************************/
#if ((defined __GNUC__) || (defined __ICCARM__))
#if (defined __GNUC__)
__attribute__ ((naked))
#endif
#if ((defined __ICCARM__))
__stackless
#endif
void PendSV_Handler(void)
{
    __asm volatile
    (
    "CPSID         i                \n" /* disable interrupts (set PRIMASK) */
    "LDR           r1,=eos_current  \n"  /* if (eos_current != 0)
    { */
    "LDR           r1,[r1,#0x00]    \n"

#if (__TARGET_ARCH_THUMB == 3)          /* Cortex-M0/M0+/M1 (v6-M, v6S-M)? */
    "CMP           r1, #0           \n"
    "BEQ           restore          \n"
    "NOP                            \n"
    "PUSH          {r4-r7}          \n" /*      push r4-r11 into stack */
    "MOV           r4, r8           \n"
    "MOV           r5, r9           \n"
    "MOV           r6, r10          \n"
    "MOV           r7, r11          \n"
    "PUSH          {r4-r7}          \n"
#else
    "CBZ           r1,restore       \n"
    "PUSH          {r4-r11}         \n"
#endif

    "LDR           r1,=eos_current  \n"  /*     eos_current->sp = sp; */
    "LDR           r1,[r1,#0x00]    \n"

#if (__TARGET_ARCH_THUMB == 3)          /* Cortex-M0/M0+/M1 (v6-M, v6S-M)? */
    "MOV           r2, SP           \n"
    "STR           r2,[r1,#0x00]    \n"  /* } */
#else
    "STR           sp,[r1,#0x00]    \n"  /* } */
#endif

    "restore: LDR r1,=eos_next      \n"  /* sp = eos_next->sp; */
    "LDR           r1,[r1,#0x00]    \n"
#if (__TARGET_ARCH_THUMB == 3)          /* Cortex-M0/M0+/M1 (v6-M, v6S-M)? */
    "LDR           r0,[r1,#0x00]    \n"
    "MOV           SP, r0           \n"
#else
    "LDR           sp,[r1,#0x00]    \n"
#endif

    "LDR           r1,=eos_next     \n"  /* eos_current = eos_next; */
    "LDR           r1,[r1,#0x00]    \n"
    "LDR           r2,=eos_current  \n"
    "STR           r1,[r2,#0x00]    \n"
#if (__TARGET_ARCH_THUMB == 3)          /* Cortex-M0/M0+/M1 (v6-M, v6S-M)? */
    "POP           {r4-r7}          \n"
    "MOV           r8, r4           \n"
    "MOV           r9, r5           \n"
    "MOV           r10,r6           \n"
    "MOV           r11,r7           \n"
    "POP           {r4-r7}          \n"
#else
    "POP           {r4-r11}         \n"  /* pop registers r4-r11 */
#endif
    "CPSIE         i                \n"  /* enable interrupts (clear PRIMASK) */
    "BX            lr               \n"   /* return to the next task */
    );
}
#endif

/* private function --------------------------------------------------------- */


static int32_t critical_count = 0;
#if (defined __CC_ARM)
inline void eos_critical_enter(void)
#elif ((defined __GNUC__) || (defined __ICCARM__))
__attribute__((always_inline)) inline void eos_critical_enter(void)
#endif
{
#if (defined __CC_ARM)
    __disable_irq();
#elif ((defined __GNUC__) || (defined __ICCARM__))
    __asm volatile ("cpsid i" : : : "memory");
#endif
    critical_count ++;
}

#if (defined __CC_ARM)
inline void eos_critical_exit(void)
#elif ((defined __GNUC__) || (defined __ICCARM__))
__attribute__((always_inline)) inline void eos_critical_exit(void)
#endif
{
    critical_count --;
    if (critical_count <= 0)
    {
        critical_count = 0;
#if (defined __CC_ARM)
        __enable_irq();
#elif ((defined __GNUC__) || (defined __ICCARM__))
        __asm volatile ("cpsie i" : : : "memory");
#endif
    }
}

#ifdef __cplusplus
}
#endif
