/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include <stdlib.h>
#include "elab_device.h"
#include "elib_hash_table.h"
#include "elab_assert.h"

/* private define ----------------------------------------------------------- */
#define EDEV_HASH_SIZE_MAX                      (128)

/* private typedef ---------------------------------------------------------- */
static elib_hash_table_t *ht_edev = NULL;
static osMutexId_t mutex_edev = NULL;
static const osMutexAttr_t mutex_attr_edev =
{
    .name = "mutex_edev",
    .attr_bits = osMutexPrioInherit | osMutexRecursive,
    .cb_mem = NULL,
    .cb_size = 0,
};

/* private function prototype ----------------------------------------------- */
elab_inline void _ht_edev_create(void);

/* public functions --------------------------------------------------------- */
void elab_device_register(elab_device_t *const me, const char *name, void *user_data)
{
    _ht_edev_create();

    osStatus_t ret_mutex = osMutexAcquire(mutex_edev, osWaitForever);
    elab_assert(ret_mutex == osOK);

    me->name = name;
    me->user_data = user_data;
    me->mutex = osMutexNew(&mutex_attr_edev);
    elab_assert(me->mutex != NULL);

    ret_mutex = osMutexRelease(mutex_edev);
    elab_assert(ret_mutex == osOK);
}

bool elab_device_valid(const char *dev_name)
{
    _ht_edev_create();

    osStatus_t ret_mutex = osMutexAcquire(mutex_edev, osWaitForever);
    elab_assert(ret_mutex == osOK);

    bool ret = elib_hash_table_existent(ht_edev, (char *)dev_name);

    ret_mutex = osMutexRelease(mutex_edev);
    elab_assert(ret_mutex == osOK);

    return ret;
}

elab_device_t *elab_device_find(const char *name)
{
    _ht_edev_create();

    osStatus_t ret_mutex = osMutexAcquire(mutex_edev, osWaitForever);
    elab_assert(ret_mutex == osOK);

    elab_device_t *device =
        (elab_device_t *)elib_hash_table_get(ht_edev, (char *)name);
    elab_assert(device != NULL);

    ret_mutex = osMutexRelease(mutex_edev);
    elab_assert(ret_mutex == osOK);

    return device;
}

void elab_device_lock(elab_device_t *const me)
{
    osStatus_t ret_mutex = osMutexAcquire(me->mutex, osWaitForever);
    elab_assert(ret_mutex == osOK);
}

void elab_device_unlock(elab_device_t *const me)
{
    osStatus_t ret_mutex = osMutexRelease(me->mutex);
    elab_assert(ret_mutex == osOK);
}

/* private functions -------------------------------------------------------- */
elab_inline void _ht_edev_create(void)
{
    if (ht_edev == NULL)
    {
        ht_edev = elib_hash_table_new(EDEV_HASH_SIZE_MAX);
        elab_assert(ht_edev != NULL);
        mutex_edev = osMutexNew(&mutex_attr_edev);
        elab_assert(mutex_edev != NULL);
    }
}

/* ----------------------------- end of file -------------------------------- */
