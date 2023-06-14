
#include "mb.h"

/*
*********************************************************************************************************
*                                          MB_ASCII_BinToHex()
*
* Description : Converts a byte into two ASCII characters into the given buffer.
*
* Argument(s) : value      The byte of data to be converted.
*               pbuf       A pointer to the buffer to store the ASCII chars.
*
* Return(s)   : The buffer pointer which has been updated to point to the next char in the buffer.
*
* Caller(s)   : MB_ASCII_Tx().
*
* Note(s)     : (1) The function ONLY converts the byte to ASCII and DOES NOT null terminate the string.
*********************************************************************************************************
*/

#if (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
uint8_t *MB_ASCII_BinToHex (uint8_t  value, uint8_t *pbuf)
{
    uint8_t nibble = (value >> 4) & 0x0F;                /* Upper Nibble                                       */
    if (nibble <= 9) {
        *pbuf++ = (uint8_t)(nibble + '0');
    } else {
        *pbuf++ = (uint8_t)(nibble - 10 + 'A');
    }

    nibble = value & 0x0F;                       /* Lower Nibble                                       */
    if (nibble <= 9) {
        *pbuf++ = (uint8_t)(nibble + '0');
    } else {
        *pbuf++ = (uint8_t)(nibble - 10 + 'A');
    }
    return (pbuf);
}
#endif

 
/*
*********************************************************************************************************
*                                          MB_ASCII_HexToBin()
*
* Description : Converts the first two ASCII hex characters in the buffer into one byte.
*
* Argument(s) : phex     Pointer to the buffer that contains the two ascii chars.
*
* Return(s)   : value of the two ASCII HEX digits pointed to by 'phex'.
*
* Caller(s)   : MB_ASCII_RxByte(),
*               MB_ASCII_Rx(),
*               MB_ASCII_RxCalcLRC(),
*               MB_ASCII_TxCalcLRC().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
uint8_t  MB_ASCII_HexToBin (uint8_t  *phex)
{
    uint8_t  value;
    uint8_t  high;
    uint8_t  low;


    high = *phex;                                /* Get upper nibble                                   */
    phex++;
    low  = *phex;                                /* Get lower nibble                                   */
    if (high <= '9') {                           /* Upper Nibble                                       */
        value  = (uint8_t)(high - '0');
    } else if (high <= 'F') {
        value  = (uint8_t)(high - 'A' + 10);
    } else {
        value  = (uint8_t)(high - 'a' + 10);
    }
    value <<= 4;

    if (low <= '9') {                            /* Lower Nibble                                       */
        value += (uint8_t)(low - '0');
    } else if (low <= 'F') {
        value += (uint8_t)(low - 'A' + 10);
    } else {
        value += (uint8_t)(low - 'a' + 10);
    }
    return (value);
}
#endif

 
/*
*********************************************************************************************************
*                                          MB_ASCII_RxCalcLRC()
*
* Description : The function calculates an 8-bit Longitudinal Redundancy Check on a MODBUS_FRAME
*               structure.
*
* Argument(s) : none.
*
* Return(s)   : The calculated LRC value.
*
* Caller(s)   : MBS_ASCII_Task().
*
* Note(s)     : (1) The LRC is calculated on the ADDR, FC and Data fields, not the ':', CR/LF and LRC
*                   placed in the message by the sender.  We thus need to subtract 5 'ASCII' characters
*                   from the received message to exclude these.
*********************************************************************************************************
*/

#if (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
uint8_t  MB_ASCII_RxCalcLRC (elab_mb_channel_t  *pch)
{
    uint8_t   lrc;
    uint16_t   len;
    uint8_t  *pblock;


    len    = (pch->RxBufByteCtr - 5) / 2 ;        /* LRC to include Addr + FC + Data                    */
    pblock = (uint8_t *)&pch->RxBuf[1];
    lrc    = 0;
    while (len-- > 0) {                          /* For each byte of data in the data block...         */
        lrc    += MB_ASCII_HexToBin(pblock);     /* Add the data byte to LRC, increment data pointer.  */
        pblock += 2;
    }

    lrc = ~lrc + 1;                              /* Two complement the binary sum                      */
    return (lrc);                                /* Return LRC for all data in block.                  */
}
#endif

 
/*
*********************************************************************************************************
*                                          MB_ASCII_TxCalcLRC()
*
* Description : The function calculates an 8-bit Longitudinal Redundancy Check on a MODBUS_FRAME
*               structure.
*
* Argument(s) : none.
*
* Return(s)   : The calculated LRC value.
*
* Caller(s)   : MB_ASCII_Tx().
*
* Note(s)     : (1) The LRC is calculated on the ADDR, FC and Data fields, not the ':' which was inserted
*                   in the TxBuf[].  Thus we subtract 1 ASCII character from the LRC.
*
*               (2) The LRC and CR/LF bytes are not YET in the .RxBuf[].
*********************************************************************************************************
*/

#if (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
uint8_t  MB_ASCII_TxCalcLRC (elab_mb_channel_t  *pch, uint16_t tx_bytes)
{
    uint8_t     lrc;
    uint16_t     len;
    uint8_t    *pblock;


    len    = (tx_bytes - 1) / 2;                 /* LRC to include Addr + FC + Data (exclude ':')      */
    pblock = (uint8_t *)&pch->TxBuf[1];
    lrc    = 0;
    while (len-- > 0) {                          /* For each byte of data in the data block...         */
        lrc    += MB_ASCII_HexToBin(pblock);     /* Add the data byte to LRC, increment data pointer.  */
        pblock += 2;
    }
    lrc = ~lrc + 1;                              /* Two complement the binary sum                      */
    return (lrc);                                /* Return LRC for all data in block.                  */
}
#endif

 
/*
*********************************************************************************************************
*                                          MB_RTU_RxCalcCRC()
*
* Description : The polynomial is a CRC-16 found for 'MBS_RxFrameNDataBytes' number of characters
*               starting at 'MBS_RxFrameAddr'.
*
* Argument(s) : none.
*
* Return(s)   : An unsigned 16-bit value representing the CRC-16 of the data.
*
* Caller(s)   : MBS_RTU_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_RTU_EN != 0)
uint16_t  MB_RTU_RxCalcCRC (elab_mb_channel_t  *pch)
{
    uint16_t      crc;
    uint8_t      shiftctr;
    bool     flag;
    uint16_t      length;
    uint8_t     *pblock;


    pblock = (uint8_t *)&pch->RxFrameData[0];      /* Starting address of where the CRC data starts            */
    length = pch->RxFrameNDataBytes + 2;              /* Include the address and function code in the CRC         */
    crc    = 0xFFFF;                                  /* Initialize CRC to all ones.                              */
    while (length > 0) {                              /* Account for each byte of data                            */
        length--;
        crc      ^= (uint16_t)*pblock++;
        shiftctr  = 8;
        do {
            flag   = (crc & 0x0001) ? true : false; /* Determine if the shift out of rightmost bit is 1   */
            crc  >>= 1;                               /* Shift CRC to the right one bit.                          */
            if (flag == true) {                   /* If (bit shifted out of rightmost bit was a 1)            */
                crc ^= MODBUS_CRC16_POLY;             /*     Exclusive OR the CRC with the generating polynomial. */
            }
            shiftctr--;
        } while (shiftctr > 0);
    }
    pch->RxFrameCRC_Calc = crc;
    return (crc);
}
#endif

 
/*
*********************************************************************************************************
*                                           MB_RTU_TxCalcCRC()
*
* Description : The polynomial is a CRC-16 found for 'MBS_TxFrameNDataBytes' number of characters
*               starting at 'MBS_TxFrameAddr'.
*
* Argument(s) : none.
*
* Return(s)   : An unsigned 16-bit value representing the CRC-16 of the data.
*
* Caller(s)   : MB_RTU_Tx().
*
* Note*(s)    : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_RTU_EN != 0)
uint16_t  MB_RTU_TxCalcCRC (elab_mb_channel_t *pch)
{
    uint16_t      crc;
    uint8_t      shiftctr;
    bool     flag;
    uint16_t      length;
    uint8_t     *pblock;


    pblock = (uint8_t *)&pch->TxFrameData[0];       /* Starting address of where the CRC data starts           */
    length = pch->TxFrameNDataBytes + 2;               /* Include the address and function code in the CRC        */
    crc    = 0xFFFF;                                   /* Initialize CRC to all ones.                             */
    while (length > 0) {                               /* Account for each byte of data                           */
        length--;
        crc      ^= (uint16_t)*pblock++;
        shiftctr  = 8;
        do {
            flag   = (crc & 0x0001) ? true : false; /* Determine if the shift out of rightmost bit is 1.  */
            crc  >>= 1;                                /* Shift CRC to the right one bit.                         */
            if (flag == true) {                    /* If (bit shifted out of rightmost bit was a 1)           */
                crc ^= MODBUS_CRC16_POLY;              /*     Exclusive OR the CRC with the generating polynomial */
            }
            shiftctr--;
        } while (shiftctr > 0);
    }
    return (crc);                                      /* Return CRC for all data in block.                       */
}
#endif
