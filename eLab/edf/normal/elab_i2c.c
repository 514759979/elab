/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_device.h"
#include "elab_assert.h"
#include "elab_i2c.h"
#include "cmsis_os.h"

#define TAG                                     "EDF_I2C"
#define ELOG_DEBUG_ENABLE                       (1)
#define ELOG_INFO_ENABLE                        (1)

#include "elab_log.h"

static const osMutexAttr_t mutex_i2c_attr =
{
    "i2c_mutex",
    osMutexPrioInherit,
    NULL,
    0U
};

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  I2C bus device register
  * @param  bus         I2C bus handle.
  * @param  bus_name    Bus name.
  * @retval see elab_err_t
  */
void elab_i2c_bus_register(elab_i2c_bus_t *bus,
                            const char *name, const elab_i2c_ops_t *ops)
{
    bus->mutex = osMutexNew(&mutex_i2c_attr);
    elab_assert(bus->mutex != NULL);
}

/**
  * @brief  i2c bus transfer msgs
  * @param  bus - the pointer of elab_i2c_bus_t
  * @param  msgs - bus name
  * @param  num - msg numbers
  * @retval transfered msgs
  */
int32_t elab_i2c_xfer(elab_i2c_bus_t *bus, elab_i2c_msg_t msgs[], uint32_t num)
{
    int32_t ret;

    if (bus->ops->master_xfer)
    {
        osMutexAcquire(bus->mutex, osWaitForever);
        ret = bus->ops->master_xfer(bus, msgs, num);
        osMutexRelease(bus->mutex);
    }
    else
    {
        elog_error("I2C bus operation not supported");
        ret = 0;
    }

    return ret;
}

/**
  * @brief  i2c bus control
  * @param  bus     the pointer of elab_i2c_bus_t
  * @param  cmd     cmd id
  * @param  arg     control arument
  * @retval see elab_err_t
  */
elab_err_t elab_i2c_control(elab_i2c_bus_t *bus, uint32_t cmd, uint32_t arg)
{
    elab_err_t ret;

    if (bus->ops->i2c_bus_control)
    {
        ret = bus->ops->i2c_bus_control(bus, cmd, arg);
    }
    else
    {
        elog_error("I2C bus operation not supported");
        ret = 0;
    }

    return ret;
}

/**
  * @brief  i2c bus master send data
  * @param  bus     the pointer of elab_i2c_bus_t
  * @param  addr    device address
  * @param  flags   i2c control flag
  * @param  buf     send buffer
  * @param  count   send size
  * @retval see elab_err_t
  */
elab_err_t elab_i2c_master_send(elab_i2c_bus_t *bus, uint16_t addr,
                                uint16_t flags, const uint8_t *buf, uint32_t count)
{
    elab_err_t ret = ELAB_OK;
    elab_i2c_msg_t msg;

    msg.addr  = addr;
    msg.flags = flags;
    msg.len   = count;
    msg.buffer = (uint8_t *)buf;

    uint32_t result = elab_i2c_xfer(bus, &msg, 1);
    if (result != 1) ret = ELAB_ERR_IO;

    return ret;
}

/**
  * @brief  i2c bus master receive data
  * @param  bus     the pointer of elab_i2c_bus_t
  * @param  addr    device address
  * @param  flags   i2c control flag
  * @param  buf     receive buffer
  * @param  count   receive size
  * @retval see elab_err_t
  */
elab_err_t elab_i2c_master_recv(elab_i2c_bus_t *bus, uint16_t addr,
                                uint16_t flags, uint8_t *buf, uint32_t count)
{
    elab_err_t ret = ELAB_OK;
    elab_i2c_msg_t msg;
    elab_assert(bus != NULL);

    msg.addr   = addr;
    msg.flags = flags | elab_i2c_RD;
    msg.len = count;
    msg.buffer = buf;

    uint32_t result = elab_i2c_xfer(bus, &msg, 1);
    if (result != 1)
    {
        ret = ELAB_ERR_IO;
    }

    return ret;
}

/* ----------------------------- end of file -------------------------------- */
