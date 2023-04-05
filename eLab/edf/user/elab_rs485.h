/**
  ******************************************************************************
  * @file    rs485.h
  * @author  Embedded Software Team @Eric Wang
  * @brief   The header file of RS485 class.
  *
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 Philips Healthcare Suzhou.
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  */

#ifndef __RS485_H
#define __RS485_H

/* Includes ------------------------------------------------------------------*/
#include "elab_def.h"
#include "elab_device.h"
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
typedef struct
{
    elab_device_t *serial;
    int32_t pin_tx_en;
    bool tx_en_high_active;

    void *user_data;
} rs485_t;

/* Exported functions --------------------------------------------------------*/
elab_err_t rs485_init(rs485_t *me, const char *name,
                        const char *serial_name,
                        const char *pin_tx_en_name,
                        bool tx_en_high_active,
                        void *user_data);
elab_device_t *rs485_get_serial(rs485_t *me);

uint32_t rs485_read(rs485_t *me, void *pbuf, uint32_t size);
uint32_t rs485_write(rs485_t *me, const void *pbuf, uint32_t size);

#endif /* __RS485_H */

/**************** (C) COPYRIGHT Philips Healthcare Suzhou ******END OF FILE****/
