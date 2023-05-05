
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include "cmsis_os.h"
#include "elab_assert.h"
#include "esh.h"

ELAB_TAG("CmsisOSPosix");

#if defined(__x86_64__)
typedef uint64_t                        elab_pointer_t;
#else
typedef uint32_t                        elab_pointer_t;
#endif

/* -----------------------------------------------------------------------------
Data structure
----------------------------------------------------------------------------- */
static const osMutexAttr_t mutex_attr_queue =
{
    "mutex_queue", 
    osMutexRecursive | osMutexPrioInherit, 
    NULL,
    0U 
};

static int get_pthread_priority(osPriority_t prio);

/* -----------------------------------------------------------------------------
OS Basic
----------------------------------------------------------------------------- */
osStatus_t osKernelInitialize(void)
{
    return osOK;
}

osStatus_t osDelay(uint32_t ticks)
{
    struct timespec ts;
    ts.tv_nsec = (ticks % 1000) * 1000000;
    ts.tv_sec = ticks / 1000;
    nanosleep(&ts, NULL);

    return osOK;
}

int32_t osKernelLock(void)
{
    // TODO
    
    return 0;
}

int32_t osKernelUnlock(void)
{
    // TODO

    return 0;
}

static uint64_t time_init = UINT64_MAX;
uint32_t osKernelGetTickCount(void)
{
    /* Get the current time. */
    struct timeval tt;
    gettimeofday(&tt, NULL);
    uint64_t time_current = (tt.tv_sec * 1000) + (tt.tv_usec / 1000);

    if (time_init == UINT64_MAX)
    {
        time_init = time_current;
    }

    return (uint32_t)(time_current - time_init);
}

uint32_t osKernelGetSysTimerCount(void)
{
    return osKernelGetTickCount();
}

int16_t elab_debug_uart_receive(void *buffer, uint16_t size);

static osMessageQueueId_t mq_ch = NULL;
static bool os_running = true;
osStatus_t osKernelStart(void)
{
    osStatus_t ret_mq = osOK;

    mq_ch = osMessageQueueNew(1024, 1, NULL);
    assert(mq_ch != NULL);

    while (os_running)
    {
        osDelay(1);
        char ch = esh_getch();
        ret_mq = osMessageQueuePut(mq_ch, &ch, 0, osWaitForever);
        assert(ret_mq == osOK);
    }

    return osOK;
}

void osKernelEnd(void)
{
    os_running = false;
}

int16_t elab_debug_uart_receive(void *buffer, uint16_t size)
{
    osStatus_t ret_mq;
    uint8_t *buff = (uint8_t *)buffer;
    uint16_t count = 0;
    for (uint16_t i = 0; i < size; i ++)
    {
        ret_mq = osMessageQueueGet(mq_ch, &buff[i], NULL, 0);
        if (ret_mq == osOK)
        {
            count ++;
        }
    } 

    return count;
}

/* -----------------------------------------------------------------------------
Thread
----------------------------------------------------------------------------- */
typedef void *(*os_pthread_func_t) (void *argument);

osThreadId_t osThreadNew(osThreadFunc_t func, void *argument, const osThreadAttr_t *attr)
{
    int ret = 0;
    pthread_t thread = 0;
    pthread_attr_t thread_attr;
    struct sched_param param;

    ret = pthread_attr_init(&thread_attr);
    assert(ret == 0);

    ret = pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
    assert(ret == 0);

    ret = pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
    assert(ret == 0);

    if (attr != NULL)
    {
        param.sched_priority = get_pthread_priority(attr->priority);
    }
    else
    {
        param.sched_priority = get_pthread_priority(osPriorityNormal);
    }
    ret = pthread_attr_setschedparam(&thread_attr, &param);
    assert(ret == 0);

    ret = pthread_create(&thread, NULL, (os_pthread_func_t)func, argument);
    assert(ret == 0);

    pthread_attr_destroy(&thread_attr);

    return (osThreadId_t)thread;
}

osThreadId_t osThreadGetId(void)
{
    return (osThreadId_t)pthread_self();
}

osStatus_t osThreadTerminate(osThreadId_t thread_id)
{
    pthread_cancel((pthread_t)thread_id);

    return osOK;
}

void osThreadExit(void)
{
    pthread_exit(NULL);
}

osStatus_t osThreadJoin(osThreadId_t thread_id)
{
    assert(thread_id != NULL);

    int ret = pthread_join((pthread_t)thread_id, NULL);
    assert(ret == 0);

    return osOK;
}

/* -----------------------------------------------------------------------------
Message queue
----------------------------------------------------------------------------- */
typedef struct os_mq
{
    osMutexId_t mutex;
    osSemaphoreId_t sem_empty;
    osSemaphoreId_t sem_full;
    uint8_t *memory;
    uint32_t msg_size;
    uint32_t capacity;
    uint32_t head;
    uint32_t tail;
    bool empty;
    bool full;
} os_mq_t;

osMessageQueueId_t osMessageQueueNew(uint32_t msg_count,
                                     uint32_t msg_size,
                                     const osMessageQueueAttr_t *attr)
{
    assert(attr == NULL);
    assert(msg_count != 0);
    assert(msg_size != 0);

    os_mq_t *mq = malloc(sizeof(os_mq_t));
    assert(mq != NULL);

    mq->mutex = osMutexNew(&mutex_attr_queue);
    assert(mq->mutex != NULL);
    mq->sem_full = osSemaphoreNew(msg_count, msg_count, NULL);
    assert(mq->sem_full != NULL);
    mq->sem_empty = osSemaphoreNew(msg_count, 0, NULL);
    assert(mq->sem_empty != NULL);

    mq->capacity = msg_count;
    mq->msg_size = msg_size;
    mq->memory = (uint8_t *)malloc(msg_size * msg_count);
    assert(mq->memory != NULL);

    mq->head = 0;
    mq->tail = 0;
    mq->empty = true;
    mq->full = false;

    return (osMessageQueueId_t)mq;
}

osStatus_t osMessageQueueDelete(osMessageQueueId_t mq_id)
{
    os_mq_t *mq = (os_mq_t *)mq_id;

    osMutexDelete(mq->mutex);
    free(mq->memory);
    free(mq);

    return osOK;
}

osStatus_t osMessageQueueReset(osMessageQueueId_t mq_id)
{
    os_mq_t *mq = (os_mq_t *)mq_id;

    osMutexAcquire(mq->mutex, osWaitForever);

    mq->head = 0;
    mq->tail = 0;
    mq->empty = true;
    mq->full = false;

    osMutexRelease(mq->mutex);

    return osOK;
}

osStatus_t osMessageQueuePut(osMessageQueueId_t mq_id,
                             const void *msg_ptr,
                             uint8_t msg_prio,
                             uint32_t timeout)
{
    (void)msg_prio;

    if (timeout != osWaitForever)
    {
        assert(timeout < (1000 * 60 * 60 * 24));
    }

    os_mq_t *mq = (os_mq_t *)mq_id;
    osSemaphoreAcquire(mq->sem_full, timeout);

    osMutexAcquire(mq->mutex, osWaitForever);
    assert(mq->full == false || mq->empty == false);
    memcpy(&mq->memory[mq->head * mq->msg_size], msg_ptr, mq->msg_size);
    mq->head = (mq->head + 1) % mq->capacity;
    mq->empty = false;
    if (mq->head == mq->tail)
    {
        mq->full = true;
    }
    osMutexRelease(mq->mutex);
    osSemaphoreRelease(mq->sem_empty);

    return osOK;
}

osStatus_t osMessageQueueGet(osMessageQueueId_t mq_id,
                             void *msg_ptr,
                             uint8_t *msg_prio,
                             uint32_t timeout)
{
    (void)msg_prio;

    if (timeout != osWaitForever)
    {
        assert(timeout < (1000 * 60 * 60 * 24));
    }

    os_mq_t *mq = (os_mq_t *)mq_id;
    osStatus_t ret = osSemaphoreAcquire(mq->sem_empty, timeout);
    if (ret == osErrorTimeout)
    {
        return osErrorTimeout;
    }

    osMutexAcquire(mq->mutex, osWaitForever);
    assert(mq->full == false || mq->empty == false);
    memcpy(msg_ptr, &mq->memory[mq->tail * mq->msg_size], mq->msg_size);
    mq->tail = (mq->tail + 1) % mq->capacity;
    mq->full = false;
    if (mq->head == mq->tail)
    {
        mq->empty = true;
    }
    osMutexRelease(mq->mutex);
    osSemaphoreRelease(mq->sem_full);

    return osOK;
}

/* -----------------------------------------------------------------------------
Mutex
----------------------------------------------------------------------------- */
typedef struct os_mutex_data
{
    osMutexAttr_t attr;
    pthread_mutex_t mutex;
} os_mutex_data_t;

osMutexId_t osMutexNew(const osMutexAttr_t *attr)
{
    (void)attr;

    os_mutex_data_t *data = malloc(sizeof(os_mutex_data_t));
    assert(data != NULL);

    int ret = pthread_mutex_init(&data->mutex, NULL);
    assert(ret == 0);

    memcpy(&data->attr, attr, sizeof(osMutexAttr_t));

    return (osMutexId_t)data;
}

osStatus_t osMutexDelete(osMutexId_t mutex_id)
{
    os_mutex_data_t *data = (os_mutex_data_t *)mutex_id;

    int ret = pthread_mutex_destroy(&data->mutex);
    assert(ret == 0);

    free(data);

    return osOK;
}

osStatus_t osMutexAcquire(osMutexId_t mutex_id, uint32_t timeout)
{
    assert(mutex_id != NULL);

    os_mutex_data_t *data = (os_mutex_data_t *)mutex_id;
    assert_name(timeout == osWaitForever, data->attr.name);

    int ret = pthread_mutex_lock(&data->mutex);
    if (ret != 0)
    {
        return osError;
    }
    else
    {
        return osOK;
    }
}

osStatus_t osMutexRelease(osMutexId_t mutex_id)
{
    os_mutex_data_t *data = (os_mutex_data_t *)mutex_id;

    int ret = pthread_mutex_unlock(&data->mutex);
    if (ret != 0)
    {
        return osError;
    }
    else
    {
        return osOK;
    }
}

/* -----------------------------------------------------------------------------
Semaphore
----------------------------------------------------------------------------- */
#include <semaphore.h>

osSemaphoreId_t osSemaphoreNew(uint32_t max_count, uint32_t initial_count, const osSemaphoreAttr_t *attr)
{
    assert(initial_count <= max_count);

    sem_t *sem = malloc(sizeof(sem_t));
    assert(sem != NULL);

    int ret = sem_init(sem, 0, max_count);
    assert(ret == 0);

    for (uint32_t i = initial_count; i < max_count; i ++)
    {
        ret = sem_wait(sem);
        assert(ret == 0);
    }

    return (osSemaphoreId_t)sem;
}

osStatus_t osSemaphoreDelete(osSemaphoreId_t semaphore_id)
{
    sem_destroy((sem_t *)semaphore_id);
    free((sem_t *)semaphore_id);

    return osOK;
}

osStatus_t osSemaphoreRelease(osSemaphoreId_t semaphore_id)
{
    int ret = sem_post((sem_t *)semaphore_id);
    // assert(ret == 0);

    return osOK;
}

osStatus_t osSemaphoreAcquire(osSemaphoreId_t semaphore_id, uint32_t timeout)
{
    assert(semaphore_id != NULL);

    if (timeout != osWaitForever)
    {
        assert(timeout < (1000 * 60 * 60 * 24));
    }

    int ret;
    if (timeout == osWaitForever)
    {
        ret = sem_wait((sem_t *)semaphore_id);
        // assert(ret == 0);
    }
    else
    {
        struct timeval tt;
        gettimeofday(&tt, NULL);

        uint32_t second = timeout / 1000;
        uint32_t ns = (timeout % 1000) * 1000 * 1000;

        struct timespec ts;
        ts.tv_sec = tt.tv_sec + second;
        ts.tv_nsec = tt.tv_usec * 1000 + ns;
        ts.tv_sec += (ts.tv_nsec / (1000 * 1000 * 1000));
        ts.tv_nsec %= (1000 * 1000 * 1000);

        ret = sem_timedwait((sem_t *)semaphore_id, &ts);
        if (ret == -1)
        {
            if (errno == ETIMEDOUT)
            {
                return osErrorTimeout;
            }
        }
    }

    return osOK;
}

bool platform_runing = true;
void os_assert_handler(const char *ex_string, const char *func, uint32_t line)
{
    printf("assert: %s, %s, %u.\n", ex_string, func, line);
    fflush(stdout);

    platform_runing = false;
}

/* private function --------------------------------------------------------- */

static int get_pthread_priority(osPriority_t prio)
{
    if (prio > osPriorityRealtime7)
    {
        prio = osPriorityRealtime7;
    }

    return (osPriorityISR + 8 - (osPriorityRealtime7 - (prio)));
}

/* ----------------------------- end of file -------------------------------- */
