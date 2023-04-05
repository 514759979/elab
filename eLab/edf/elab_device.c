/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_assert.h"
#include "elab_device.h"
#include "elab_def.h"
#include "cmsis_os.h"
#include "elab_device_def.h"

#define TAG                                 "DEVICE"
#define ELOG_DEBUG_ENABLE                   (1)
#define ELOG_INFO_ENABLE                    (1)

#include "elab_log.h"

/* private function prototypes ---------------------------------------------- */
static void _add_device(elab_device_t *me);
static osMutexId_t _device_mutex(void);

/* private variables -------------------------------------------------------- */
static elab_device_t *edev_table[ELAB_DEV_NUM_MAX];
static uint32_t edev_device_count = 0;
static osMutexId_t mutex_edev = NULL;

static const osMutexAttr_t mutex_attr_edev =
{
    .name = "mutex_edev",
    .attr_bits = osMutexPrioInherit | osMutexRecursive,
    .cb_mem = NULL,
    .cb_size = 0,
};

/**
 * This function registers a device driver with specified name.
 *
 * @param dev the pointer of device driver structure
 * @param name the device driver's name
 *
 * @return the error code, ELAB_OK on initialization successfully.
 */
void elab_device_register(elab_device_t *me, elab_device_attr_t *attr)
{
    elab_assert(me != NULL);
    elab_assert(attr != NULL);
    elab_assert(attr->name != NULL);
    elab_assert_name(elab_device_find(attr->name) == NULL, attr->name);

    osMutexId_t mutex = _device_mutex();
    
    osMutexAcquire(mutex, osWaitForever);
    memcpy(&me->attr, attr, sizeof(elab_device_attr_t));
    me->enable_count = 0;

    me->mutex = osMutexNew(&mutex_attr_edev);
    elab_assert(me->mutex != NULL);

    _add_device(me);

    osMutexRelease(mutex);
}

/**
 * This function finds a device driver by specified name.
 *
 * @param name the device driver's name
 *
 * @return the registered device driver on successful, or NULL on failure.
 */
elab_device_t *elab_device_find(const char *name)
{
    elab_assert(name != NULL);
    
    elab_device_t *me = NULL;

    osMutexId_t lev = _device_mutex();
    osMutexAcquire(lev, osWaitForever);
    for (uint32_t i = 0; i < ELAB_DEV_NUM_MAX; i++)
    {
        if (!strcmp(edev_table[i]->attr.name, name))
        {
            me = edev_table[i];
            break;
        }
    }
    osMutexRelease(lev);

    return me;
}

/**
 * This function will open a device
 *
 * @param dev the pointer of device driver structure
 *
 * @return the result
 */
elab_err_t elab_device_enable(elab_device_t *me, bool status)
{
    elab_assert(me != NULL);
    elab_assert(me->ops != NULL);
    elab_assert(me->ops->enable != NULL);

    osMutexAcquire(me->mutex, osWaitForever);
    
    if (me->attr.sole)
    {
        elab_assert(me->enable_count == 0);
    }
    else
    {
        elab_assert(me->enable_count < UINT8_MAX);
    }
    
    elab_err_t ret = me->ops->enable(me, status);
    me->enable_count ++;
    elab_assert(me->enable_count == 0);

    osMutexRelease(me->mutex);

    return ret;
}

/**
 * This function will read some data from a device.
 *
 * @param dev       the pointer of device driver structure
 * @param buffer    the data buffer to save read data
 * @param size      the size of buffer
 *
 * @return the actually read size on successful, otherwise negative returned.
 *
 */
int32_t elab_device_read(elab_device_t *me,
                            uint32_t pos, void *buffer, uint32_t size)
{
    elab_assert(me != NULL);
    elab_assert(me->enable_count != 0);
    elab_assert(me->ops != NULL);
    elab_assert(me->ops->read != NULL);

    return me->ops->read(me, pos, buffer, size);
}

/**
 * This function will write some data to a device.
 *
 * @param me        the pointer of device driver structure.
 * @param buffer    the data buffer to be written to device
 * @param size      the size of buffer
 *
 * @return The actually written size on successful, otherwise negative returned.
 *
 */
int32_t elab_device_write(elab_device_t *me,
                            uint32_t pos, const void *buffer, uint32_t size)
{
    elab_assert(me != NULL);
    elab_assert(me->enable_count != 0);
    elab_assert(me->ops != NULL);
    elab_assert(me->ops->write != NULL);

    return me->ops->write(me, pos, buffer, size);
}

/* private functions -------------------------------------------------------- */
static osMutexId_t _device_mutex(void)
{
    if (mutex_edev == NULL)
    {
        mutex_edev = osMutexNew(&mutex_attr_edev);
        elab_assert(mutex_edev != NULL);
    }

    return mutex_edev;
}

static void _add_device(elab_device_t *me)
{
    elab_assert(edev_device_count < ELAB_DEV_NUM_MAX);

    edev_table[edev_device_count ++] = me;
}

/* ----------------------------- end of file -------------------------------- */
