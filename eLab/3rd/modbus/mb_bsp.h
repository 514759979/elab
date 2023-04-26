/**
  ******************************************************************************
  * @file    mb_bsp.h
  * @author  Embedded Software Team @Eric Wang
  * @brief   The header file for mb bsp data structure definitions.
  *
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 Philips Healthcare Suzhou.
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  */

#ifndef MB_BSP_H
#define MB_BSP_H

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <stdlib.h>
#include "mb.h"

/* public define -------------------------------------------------------------*/
enum mb_dev_type
{
    MB_DEV_TYPE_UART = 0,
    MB_DEV_TYPE_RS485,
    MB_DEV_TYPE_RS422,
};

/* Exported typedef ----------------------------------------------------------*/
typedef struct mb_dev_info
{
    uint8_t type;
    const char *serial_name;
    const char *pin_tx_en;
    const char *pin_rx_en;
    bool tx_en_high_active;
    bool rx_en_high_active;
} mb_dev_info_t;

#endif  /* MB_BSP_H */

/********** (C) COPYRIGHT Philips Healthcare Suzhou ***** END OF FILE *********/