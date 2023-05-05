/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_serial.h"
#include "elab_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("Edf_Serial");

/* private function prototypes ---------------------------------------------- */
static elab_err_t _device_enable(elab_device_t *me, bool status);
static int32_t _device_read(elab_device_t *me,
                                uint32_t pos, void *buffer, uint32_t size);
static int32_t _device_write(elab_device_t *me,
                                uint32_t pos, const void *buffer, uint32_t size);

#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
static void _device_poll(elab_device_t *me);
#endif

/* private variables -------------------------------------------------------- */
static const elab_dev_ops_t _device_ops =
{
    .enable = _device_enable,
    .read = _device_read,
    .write = _device_write,
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
    .poll = _device_poll,
#endif
};

static const osMutexAttr_t _mutex_attr =
{
    "mutex_serial_half_duplex", osMutexRecursive | osMutexPrioInherit, NULL, 0U 
};

/* public functions --------------------------------------------------------- */
/**
  * @brief  Register elab serial device to edf framework.
  * @param  serial      elab serial device handle.
  * @param  name        Serial name.
  * @param  ops         Serial device ops.
  * @param  attr        Serial device attribute.
  * @param  user_data   The pointer of private data.
  * @retval None.
  */
void elab_serial_register(elab_serial_t *serial, const char *name,
                            elab_serial_ops_t *ops,
                            elab_serial_attr_t *attr,
                            void *user_data)
{
    assert(serial != NULL);
    assert(name != NULL);
    assert(ops != NULL);

    /* The serial class data */
    serial->ops = ops;
    serial->is_sending = false;
    if (attr == NULL)
    {
        serial->attr = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;
    }
    else
    {
        memcpy(&serial->attr, attr, sizeof(elab_serial_attr_t));
    }
    
    /* Apply buffer memory */
    serial->queue_rx = osMessageQueueNew(serial->attr.rx_bufsz, 1, NULL);
    assert(serial->queue_rx != NULL);
    serial->queue_tx = osMessageQueueNew(serial->attr.tx_bufsz, 1, NULL);
    assert(serial->queue_tx != NULL);

    /* The super class data */
    elab_device_t *device = &(serial->super);
    device->ops = &_device_ops;
    device->user_data = user_data;

    /* register a character device */
    elab_device_attr_t _dev_attr =
    {
        .name = name,
        .sole = true,
        .type = ELAB_DEVICE_UART,
    };
    elab_device_register(device, &_dev_attr);
}

#ifndef __linux__
/**
  * @brief  The serial device rx ISR function.
  * @param  serial      elab serial device handle.
  * @param  buffer      The buffer memory.
  * @param  size        The serial memory size.
  * @retval None.
  */
void elab_serial_isr_rx(elab_serial_t *serial, void *buffer, uint32_t size)
{
    /* TODO not completed. */
    assert(false);
}

/**
  * @brief  The serial device tx ISR function.
  * @param  serial      elab serial device handle
  * @retval None.
  */
void elab_serial_isr_tx_end(elab_serial_t *serial)
{
    /* TODO not completed. */
    assert(false);
}
#endif

/**
  * @brief  Set the mode of elab serial device.
  * @param  serial      elab serial device handle
  * @param  mode        Serial mode, full duplex or half duplex.
  * @retval None.
  */
void elab_serial_set_mode(elab_serial_t *serial, uint8_t mode)
{
    assert(serial != NULL);
    assert(mode == ELAB_SERIAL_MODE_FULL_DUPLEX ||
            mode == ELAB_SERIAL_MODE_HALF_DUPLEX);


    elab_serial_attr_t attr;
    elab_device_lock(serial);
    attr = elab_serial_get_attr(serial);
    attr.mode = mode;
    elab_serial_set_attr(serial, &attr);
    elab_device_unlock(serial);
}

/**
  * @brief  Set the attribute of the elab serial device
  * @param  serial      elab serial device handle
  * @param  name        Serial name
  * @param  ops         Serial device ops
  * @param  user_data   The pointer of private data
  * @retval See elab_err_t
  */
void elab_serial_set_attr(elab_serial_t *serial, elab_serial_attr_t *attr)
{
    assert(serial != NULL);

    elab_device_lock(serial);
    memcpy(&serial->attr, attr, sizeof(elab_serial_attr_t));
    elab_device_unlock(serial);
}

/**
  * @brief  Register elab serial device to serial device
  * @param  serial      elab serial device handle
  * @param  name        Serial name
  * @param  ops         Serial device ops
  * @param  user_data   The pointer of private data
  * @retval See elab_err_t
  */
elab_serial_attr_t elab_serial_get_attr(elab_serial_t *serial)
{
    assert(serial != NULL);

    elab_serial_attr_t attr;

    elab_device_lock(serial);
    memcpy(&attr, &serial->attr, sizeof(elab_serial_attr_t));
    elab_device_unlock(serial);

    return attr;
}

/**
  * @brief  elab serial device xfer function, just for half duplex mode.
  * @param  serial      elab serial device handle
  * @param  name        Serial name
  * @param  ops         Serial device ops
  * @param  user_data   The pointer of private data
  * @retval See elab_err_t
  */
int32_t elab_serial_xfer(elab_device_t *me,
                            void *buff_tx, uint32_t size_tx,
                            void *buff_rx, uint32_t size_rx)
{
    assert(me != NULL);
    assert(ELAB_SERAIL_CAST(me)->ops != NULL);
    assert(ELAB_SERAIL_CAST(me)->ops->write != NULL);

    elab_serial_t *serial = ELAB_SERAIL_CAST(me);
    assert(serial->attr.mode == ELAB_SERIAL_MODE_HALF_DUPLEX);

    elab_device_lock(serial);

    int32_t ret = 0;

#ifdef __linux__
    serial->is_sending = true;
    serial->ops->write(serial, buff_tx, size_tx);
    ret = serial->ops->read(serial, buff_rx, size_rx);
#else
    /* TODO Not completed. */
    assert(false);
#endif

    elab_device_unlock(serial);

    return ret;
}

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  elab device enable function
  * @param  me  The elab device handle.
  * @retval See elab_err_t
  */
static elab_err_t _device_enable(elab_device_t *me, bool status)
{
    elab_err_t ret = ELAB_OK;

    assert(me != NULL);

    elab_serial_t *serial = (elab_serial_t *)me;
    assert(serial->ops != NULL);
    assert(serial->ops->enable != NULL);

    return serial->ops->enable(serial, status);;
}

/**
  * @brief  elab device read function
  * @param  me      The elab device handle.
  * @param  pos     Position
  * @param  buffer  The pointer of buffer
  * @param  size    Expected read length
  * @retval Auctual read length
  */
static int32_t _device_read(elab_device_t *me,
                            uint32_t pos, void *buffer, uint32_t size)
{
    assert(me != NULL);
    assert(ELAB_SERAIL_CAST(me)->ops != NULL);
    assert(ELAB_SERAIL_CAST(me)->ops->read != NULL);

    return ELAB_SERAIL_CAST(me)->ops->read(ELAB_SERAIL_CAST(me), buffer, size);
}

/**
  * @brief  elab device write function.
  * @param  me      The elab device handle.
  * @param  pos     Position
  * @param  buffer  The pointer of buffer
  * @param  size    Expected write length
  * @retval Auctual write length
  */
static int32_t _device_write(elab_device_t *me,
                                uint32_t pos, const void *buffer, uint32_t size)
{
    assert(me != NULL);
    assert(ELAB_SERAIL_CAST(me)->ops != NULL);
    assert(ELAB_SERAIL_CAST(me)->ops->write != NULL);

    return ELAB_SERAIL_CAST(me)->ops->write(ELAB_SERAIL_CAST(me), buffer, size);
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
