/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_device.h"
#include "elab_assert.h"
#include "elab_i2c.h"
#include "cmsis_os.h"
#include "elab_log.h"

ELAB_TAG("Edf_I2C");

/* Private variables ---------------------------------------------------------*/
/**
 * @brief  The I2C oprations function.
 */
static const elab_dev_ops_t i2c_ops =
{
    .enable = NULL,
    .read = NULL,
    .write = NULL,
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
    .poll = NULL,
#endif
};

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  I2C bus device register
  * @param  bus         I2C bus handle.
  * @param  bus_name    Bus name.
  * @retval see elab_err_t
  */
void elab_i2c_bus_register(elab_i2c_bus_t *bus,
                            const char *name, const elab_i2c_bus_ops_t *ops,
                            void *user_data)
{
    elab_assert(bus != NULL);
    elab_assert(name != NULL);
    elab_assert(ops != NULL);

    elab_device_t *device = &bus->super;

    /* initialize device interface. */
    device->ops = &i2c_ops;
    bus->ops = ops;
    bus->super.user_data = user_data;
    bus->config.addr_10bit = true;
    bus->config.clock = UINT32_MAX;

    /* Initialize mutex mutex */
    static const osMutexAttr_t mutex_i2c_attr =
    {
        "i2c_bus_mutex",
        osMutexPrioInherit,
        NULL,
        0U
    };
    bus->mutex = osMutexNew(&mutex_i2c_attr);
    elab_assert(NULL != bus->mutex);

    /* register to device manager */
    elab_device_attr_t attr_i2c_bus =
    {
        .name = name,
        .sole = false,
        .type = ELAB_DEVICE_I2C_BUS,
    };
    elab_device_register(device, &attr_i2c_bus);
}

/**
  * @brief  I2C bus device register
  * @param  bus         I2C bus handle.
  * @param  bus_name    Bus name.
  * @retval see elab_err_t
  */
void elab_i2c_register(elab_i2c_t *device, const char *name, const char *bus_name,
                            elab_i2c_config_t config,
                            void *user_data)
{
    elab_assert(device != NULL);
    elab_assert(name != NULL);
    elab_assert(bus_name != NULL);
 
    elab_i2c_bus_t *bus = (elab_i2c_bus_t *)elab_device_find(bus_name);
    elab_assert(bus != NULL);
    elab_assert(bus->super.attr.type == ELAB_DEVICE_I2C_BUS);

    device->bus = bus;
    device->config = config;
    device->super.user_data = user_data;

    /* Register to device manager. */
    elab_device_attr_t attr_i2c =
    {
        .name = name,
        .sole = true,
        .type = ELAB_DEVICE_I2C,
    };
    elab_device_register(&device->super, &attr_i2c);
}

/**
  * @brief  i2c bus transfer msgs
  * @param  bus     the pointer of elab_i2c_bus_t
  * @param  msgs    bus name
  * @param  num     msg numbers
  * @retval transfered msgs
  */
int32_t elab_i2c_xfer_msgs(elab_device_t *me, elab_i2c_msg_t msgs[], uint32_t num)
{
    elab_assert(me != NULL);
    elab_assert(me->attr.type == ELAB_DEVICE_I2C);

    int32_t ret = (int32_t)ELAB_OK;
    osStatus_t ret_os = osOK;
    elab_i2c_t *i2c = (elab_i2c_t *)me;

    elab_assert(i2c->bus != NULL);

    ret_os = osMutexAcquire(i2c->bus->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    /* If not the same config as current, re-configure i2c bus */
    if (memcmp(&i2c->bus->config,
                &i2c->config, sizeof(elab_i2c_bus_config_t)) != 0)
    {
        ret = i2c->bus->ops->config(i2c->bus, (elab_i2c_bus_config_t *)&i2c->config);
        if (ret != ELAB_OK)
        {
            goto exit;
        }

        /* Set i2c bus's config. */
        memcpy(&i2c->bus->config, &i2c->config, sizeof(elab_i2c_bus_config_t));
    }

    /* Transfer message. */
    for (uint32_t i = 0; i < num; i ++)
    {
        ret = i2c->bus->ops->xfer(i2c->bus, i2c->config.addr, msgs[i]);
        if (ret != ELAB_OK)
        {
            goto exit;
        }
    }

    ret = num;
    
exit:
    ret_os = osMutexRelease(i2c->bus->mutex);
    elab_assert(ret_os == osOK);
    (void)ret_os;

    return ret;
}

/**
  * @brief  i2c bus transfer msgs
  * @param  bus     the pointer of elab_i2c_bus_t
  * @param  msgs    bus name
  * @param  num     msg numbers
  * @retval transfered msgs
  */
elab_err_t elab_i2c_xfer(elab_device_t *me, elab_i2c_msg_t msg)
{
    elab_assert(me != NULL);
    elab_assert(me->attr.type == ELAB_DEVICE_I2C);

    elab_err_t ret = ELAB_OK;
    osStatus_t ret_os = osOK;
    elab_i2c_t *i2c = (elab_i2c_t *)me;

    elab_assert(i2c->bus != NULL);

    ret_os = osMutexAcquire(i2c->bus->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    /* If not the same config as current, re-configure i2c bus */
    if (memcmp(&i2c->bus->config,
                &i2c->config, sizeof(elab_i2c_bus_config_t)) != 0)
    {
        ret = i2c->bus->ops->config(i2c->bus, (elab_i2c_bus_config_t *)&i2c->config);
        if (ret != ELAB_OK)
        {
            goto exit;
        }

        /* Set i2c bus's config. */
        memcpy(&i2c->bus->config, &i2c->config, sizeof(elab_i2c_bus_config_t));
    }

    /* Transfer message. */
    ret = (elab_err_t)i2c->bus->ops->xfer(i2c->bus, i2c->config.addr, msg);

exit:
    ret_os = osMutexRelease(i2c->bus->mutex);
    elab_assert(ret_os == osOK);
    (void)ret_os;

    return ret;
}

/* ----------------------------- end of file -------------------------------- */
