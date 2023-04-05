/**
  ******************************************************************************
  * @file    rs485.c
  * @author  Embedded Software Team @Eric Wang
  * @brief   The source file of RS485 class.
  *
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 Philips Healthcare Suzhou.
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "elab_rs485.h"
#include "elab_serial.h"
#include "elab_pin.h"
#include <stdbool.h>

#define TAG                                     "rs485"
#define ELOG_DEBUG_ENABLE                      (1)
#define ELOG_INFO_ENABLE                       (1)

#include "elab_log.h"
#include "elab_assert.h"

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

    me->pin_tx_en = elab_pin_get(pin_tx_en_name);
    if (ELAB_ERR_NO_SYSTEM == me->pin_tx_en)
    {
        elog_error("Pin device %s not found.", pin_tx_en_name);
        ret = ELAB_ERROR;
        goto exit;
    }

    me->tx_en_high_active = tx_en_high_active;
    me->user_data = user_data;

    // Serial mode setting.
    elab_io_uart_mode serial_mode = ELAB_IO_UART_MODE_BLOCKING;
    elab_err_t ret_mode =
    elab_io_control(me->serial,
                            ELAB_IO_CTRL_UART_SET_MODE,
                            &serial_mode);
    if (ELAB_OK != ret_mode)
    {
        elog_error("rs485 serial %s mode setting fails. Err ID: %d.",
                     serial_name,
                     ret_mode);
        ret = ELAB_ERROR;
        goto exit;
    }

    // Set the rx485 to receiving mode.
    elab_pin_mode(me->pin_tx_en, PIN_MODE_OUTPUT);
    rs485_tx_active(me, false);

    // Serail port opening.
    ret = elab_io_open(me->serial);
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
    elab_assert(serial->uart_mode == ELAB_IO_UART_MODE_BLOCKING);

    return elab_io_read(me->serial, 0, pbuf, size);
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
    elab_assert(serial->uart_mode == ELAB_IO_UART_MODE_BLOCKING);

    // Set the rx485 to sending mode.
    rs485_tx_active(me, true);

    uint32_t ret = elab_io_write(me->serial, 0, pbuf, size);

    // Set the rx485 to receiving mode.
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
    int32_t tx_en_status;
    if (me->tx_en_high_active == true)
    {
        tx_en_status = active ? 1 : 0;
    }
    else
    {
        tx_en_status = active ? 0 : 1;
    }

    elab_pin_write(me->pin_tx_en, tx_en_status);
}

/**************** (C) COPYRIGHT Philips Healthcare Suzhou ******END OF FILE****/
