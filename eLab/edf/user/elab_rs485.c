/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include "elab_rs485.h"
#include "elab_serial.h"
#include "elab_pin.h"
#include <stdbool.h>
#include "elab_log.h"
#include "elab_assert.h"

ELAB_TAG("rs485");

/* Private function prototypes -----------------------------------------------*/
static void rs485_tx_active(rs485_t *me, bool active);

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  rs485 init function
  * @param  me The RS485 handle
  * @param  serial_name The input serial device name
  * @param  pin_tx_en_name  The input tx pin name
  * @param  tx_en_high_active The tx enable pin is high-active or not.
  * @param  user_data   User private data attached to the rs485 object.
  * @retval See elab_err_t
  */
elab_err_t rs485_init(rs485_t *me,
                            const char *serial_name,
                            const char *pin_tx_en_name,
                            bool tx_en_high_active,
                            void *user_data)
{
    elab_err_t ret = ELAB_OK;

    me->serial = elab_device_find(serial_name);
    if (NULL == me->serial)
    {
        elog_error("Serial device %s not found.", serial_name);
        ret = ELAB_ERROR;
        goto exit;
    }

    me->pin_tx_en = elab_device_find(pin_tx_en_name);
    if (NULL == me->pin_tx_en)
    {
        elog_error("PIN device %s not found.", pin_tx_en_name);
        ret = ELAB_ERROR;
        goto exit;
    }

    me->tx_en_high_active = tx_en_high_active;
    me->user_data = user_data;

    /* Serial mode setting. */
    elab_serial_set_mode((elab_serial_t *)me->serial,
                            (uint8_t)ELAB_SERIAL_MODE_HALF_DUPLEX);

    /* Set the rx485 to receiving mode. */
    elab_pin_set_mode(me->pin_tx_en, PIN_MODE_OUTPUT_PP);
    rs485_tx_active(me, false);

    /* Serail port opening. */
    ret = elab_device_open(me->serial);
    if (ELAB_OK != ret)
    {
        elog_error("RS485 opening serial %s port fails.", serial_name);
        goto exit;
    }

exit:
    return ret;
}

/**
  * @brief  Get serial handle from RS485.
  * @param  me   The RS485 handle
  * @retval Serail handle.
  */
elab_device_t *rs485_get_serial(rs485_t *me)
{
    return me->serial;
}

/**
  * @brief  RS485 read function
  * @param  me   The RS485 handle
  * @param  pbuf The pointer of buffer
  * @param  size Expected read length
  * @retval Auctual read length
  */
uint32_t rs485_read(rs485_t *me, void *pbuf, uint32_t size)
{
    elab_serial_t *serial = (elab_serial_t *)me->serial;
    elab_assert(serial->attr.mode == ELAB_SERIAL_MODE_HALF_DUPLEX);

    return elab_device_read(me->serial, 0, pbuf, size);
}

/**
  * @brief  RS485 write function
  * @param  me   The RS485 handle
  * @param  pbuf The pointer of buffer
  * @param  size Expected write length
  * @retval Auctual write length
  */
uint32_t rs485_write(rs485_t *me, const void *pbuf, uint32_t size)
{
    elab_serial_t *serial = (elab_serial_t *)me->serial;
    elab_assert(serial->attr.mode == ELAB_SERIAL_MODE_HALF_DUPLEX);

    /* Set the rx485 to sending mode. */
    rs485_tx_active(me, true);

    uint32_t ret = elab_device_write(me->serial, 0, pbuf, size);

    /* Set the rx485 to receiving mode. */
    rs485_tx_active(me, false);

    return ret;
}

/* Private defines -----------------------------------------------------------*/
/**
  * @brief  RS485 set the tx mode active or not.
  * @param  me      The RS485 handle
  * @param  active  TX mode is active or not.
  * @retval None.
  */
static void rs485_tx_active(rs485_t *me, bool active)
{
    // Write the tx_en pin.
    bool tx_en_status = false;
    if (me->tx_en_high_active == true)
    {
        tx_en_status = active ? true : false;
    }
    else
    {
        tx_en_status = active ? false : true;
    }

    elab_pin_set_status(me->pin_tx_en, tx_en_status);
}

/* ----------------------------- end of file -------------------------------- */
