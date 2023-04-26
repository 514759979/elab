/**
 *******************************************************************************
 * @file    platform_crc.h
 * @author  Embedded Software Team @Eric WANG
 * @brief   The crc functions.
 *              + crc32 function.
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 Philips Healthcare (Suzhou).
 * All rights reserved.</center></h2>
 *
 *******************************************************************************
 */

#ifndef PLATFORM_API_PLATFORM_CRC_H_
#define PLATFORM_API_PLATFORM_CRC_H_

#include <stdint.h>
#include "elab_def.h"

/**
  * @brief  Private function calculating CRC32 value of one memory.
  * @param  crc         : The former CRC32 value.
  * @param  data        : Data buffer to be calculated.
  * @param  size        : Size of the buffer.
  * @retval             : CRC32 value.
  */
uint32_t elib_crc32(uint32_t crc, void *data, uint32_t size);

#endif

/********** (C) COPYRIGHT Philips Healthcare Suzhou ***** END OF FILE *********/
