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
#include "elab_log.h"
#include "elab_pin.h"

ELAB_TAG("EdfDevice");

/* private function prototypes ---------------------------------------------- */
static void _add_device(elab_device_t *me);
static osMutexId_t _edf_mutex(void);

/* private variables -------------------------------------------------------- */
static elab_device_t *_edf_table[ELAB_DEV_NUM_MAX];
static uint32_t _edf_device_count = 0;
static osMutexId_t _mutex_edf = NULL;

/**
 * The edf global mutex attribute.
 */
static const osMutexAttr_t _mutex_attr_edf =
{
    .name = "mutex_edf",
    .attr_bits = osMutexPrioInherit | osMutexRecursive,
    .cb_mem = NULL,
    .cb_size = 0,
};

/* public function ---------------------------------------------------------- */
/**
 * This function registers a device driver with specified name.
 * @param dev the pointer of device driver structure
 * @param name the device driver's name
 * @return None.
 */
void elab_device_register(elab_device_t *me, elab_device_attr_t *attr)
{
    if (_edf_device_count == 0)
    {
        for (uint32_t i = 0; i < ELAB_DEV_NUM_MAX; i ++)
        {
            _edf_table[i] = NULL;
        }
    }

    assert(me != NULL);
    assert(attr != NULL);
    assert(attr->name != NULL);
    assert_name(elab_device_find(attr->name) == NULL, attr->name);

    /* Edf mutex locking. */
    osStatus_t ret = osOK;
    osMutexId_t mutex = _edf_mutex();
    ret = osMutexAcquire(mutex, osWaitForever);
    assert(ret == osOK);

    /* Set the device data. */
    memcpy(&me->attr, attr, sizeof(elab_device_attr_t));
    me->enable_count = 0;
    me->lock_count = 0;
    me->mutex = osMutexNew(&_mutex_attr_edf);
    assert(me->mutex != NULL);

    /* Add the device the edf table. */
    _add_device(me);

    /* Edf mutex unlocking. */
    ret = osMutexRelease(mutex);
    assert(ret == osOK);
}

/**
 * This function finds a device driver by specified name.
 * @param name  Device name.
 * @return Device handle. If not found, return NULL.
 */
elab_device_t *elab_device_find(const char *name)
{
    assert(name != NULL);
    
    /* Edf mutex locking. */
    osStatus_t ret = osOK;
    osMutexId_t mutex = _edf_mutex();
    ret = osMutexAcquire(mutex, osWaitForever);
    assert(ret == osOK);

    /* Find the device */
    elab_device_t *me = NULL;
    for (uint32_t i = 0; i < ELAB_DEV_NUM_MAX; i++)
    {
        /* No device yet in the device table. */
        if (_edf_table[i] == NULL)
        {
            break;
        }
        /* Device matching */
        if (!strcmp(_edf_table[i]->attr.name, name))
        {
            me = _edf_table[i];
            break;
        }
    }

    /* Edf mutex unlocking. */
    ret = osMutexRelease(mutex);
    assert(ret == osOK);

    return me;
}

/**
 * This function check one device name is valid or not.
 * @param name  Device name.
 * @return Valid if true and invalid if false.
 */
bool elab_device_valid(const char *name)
{
    return elab_device_find(name) == NULL ? false : true;
}

/**
 * Lock or unlock one device to ensure its thread-safety.
 * @param me        Device handle.
 * @param status    The locking status.
 * @return None.
 */
void __device_mutex_lock(elab_device_t *me, bool status)
{
    osStatus_t ret = osOK;

    if (status)
    {
        if (me->lock_count == 0)
        {
            ret = osMutexAcquire(me->mutex, osWaitForever);
            assert(ret == osOK);
        }

        me->lock_count ++;
    }
    else
    {
        me->lock_count --;
        if (me->lock_count == 0)
        {
            ret = osMutexRelease(me->mutex);
            assert(ret == osOK);
        }
    }
}

/**
 * This function will enable or disable a device
 * @param me        Device handle.
 * @param status    The locking status.
 * @return the result
 */
elab_err_t __device_enable(elab_device_t *me, bool status)
{
    assert(me != NULL);
    assert(me->ops != NULL);
    assert(me->ops->enable != NULL);

    elab_device_lock(me);
    
    if (me->attr.sole)
    {
        if (status)
        {
            assert_name(me->enable_count == 0, me->attr.name);
        }
        else
        {
            assert_name(me->enable_count > 0, me->attr.name);
        }
    }
    else
    {
        assert_name(me->enable_count < UINT8_MAX, me->attr.name);
    }
    
    elab_err_t ret = me->ops->enable(me, status);
    if (status)
    {
        me->enable_count ++;
    }
    else
    {
        assert(me->enable_count > 0);
        me->enable_count --;
    }

    elab_device_unlock(me);

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
    assert(me != NULL);
    assert(me->enable_count != 0);
    assert(me->ops != NULL);
    assert(me->ops->read != NULL);

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
    assert(me != NULL);
    assert(me->enable_count != 0);
    assert(me->ops != NULL);
    assert(me->ops->write != NULL);

    return me->ops->write(me, pos, buffer, size);
}

/* private functions -------------------------------------------------------- */
static osMutexId_t _edf_mutex(void)
{
    if (_mutex_edf == NULL)
    {
        _mutex_edf = osMutexNew(&_mutex_attr_edf);
        assert(_mutex_edf != NULL);
    }

    return _mutex_edf;
}

static void _add_device(elab_device_t *me)
{
    assert(_edf_device_count < ELAB_DEV_NUM_MAX);

    if (_edf_device_count == 0)
    {
        for (uint32_t i = 0; i < ELAB_DEV_NUM_MAX; i ++)
        {
            _edf_table[i] = NULL;
        }
    }
    _edf_table[_edf_device_count ++] = me;
}

/* ----------------------------- end of file -------------------------------- */
