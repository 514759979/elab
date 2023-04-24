/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_serial.h"
#include "elab_assert.h"

/* private function prototypes ---------------------------------------------- */
static elab_err_t _device_enable(elab_device_t *me, bool status);
static int32_t _device_read(elab_device_t *me,
                                uint32_t pos, void *buffer, uint32_t size);
static int32_t _device_write(elab_device_t *me,
                                uint32_t pos, const void *buffer, uint32_t size);
static void _device_poll(elab_device_t *me);

/* private variables -------------------------------------------------------- */
static const elab_dev_ops_t _device_ops =
{
    .enable = _device_enable,
    .read = _device_read,
    .write = _device_write,
#if (ELAB_DEV_PALTFORM == 1)
    .poll = _device_poll,
#endif
};

/* public functions ----------------------------------------------------------*/

/**
  * @brief  Register uart device to serial device
  * @param  serial      The pointer of platform uart dev
  * @param  name        Device name
  * @param  user_data   The pointer of private data
  * @retval See elab_err_t
  */
void elab_serial_register(elab_serial_t *serial, const char *name,
                            elab_serial_ops_t *ops, void *user_data)
{
    elab_serial_attr_t attr = ELAB_SERIAL_ATTR_DEFAULT;

    assert(serial != NULL);
    serial->ops = ops;

    elab_device_t *device = &(serial->super);

    serial->attr = attr;

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

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  platform device open function
  * @param  pdev The pointer of platform dev
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
  * @brief  platform uart read function
  * @param  pdev The pointer of platform dev
  * @param  pos  Position
  * @param  pbuf The pointer of buffer
  * @param  size Expected read length
  * @retval Auctual read length
  */
static int32_t _device_read(elab_device_t *me,
                            uint32_t pos, void *pbuf, uint32_t size)
{
    uint32_t read_cnt = 0;

    assert(me != NULL);

    elab_serial_t *serial = (elab_serial_t *)me;
    assert(serial->ops != NULL);
    assert(serial->ops->read != NULL);

    return serial->ops->read(serial, pbuf, size);
}

/**
  * @brief  serial device write function.
  * @param  serial The pointer of serial device.
  * @param  pos  Position
  * @param  pbuf The pointer of buffer
  * @param  size Expected write length
  * @retval Auctual write length
  */
static int32_t _device_write(elab_device_t *me,
                                uint32_t pos, const void *buffer, uint32_t size)
{
    uint32_t write_cnt = 0;

    assert(me != NULL);

    elab_serial_t *serial = (elab_serial_t *)me;
    assert(serial->ops != NULL);
    assert(serial->ops->write != NULL);

    return serial->ops->write(serial, buffer, size);
}

/* ----------------------------- end of file -------------------------------- */
