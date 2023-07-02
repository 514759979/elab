/*
*********************************************************************************************************
*                                              uC/Modbus
*                                       The Embedded Modbus Stack
*
*                    Copyright 2003-2020 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        uC/MODBUS Source Code
*
* Filename : mb.c
* Version  : V2.14.00
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define    MB_MODULE

#include <stdlib.h>
#include "../../common/elab_assert.h"
#include "mb.h"

static uint8_t MB_ChCtr = 0;

ELAB_TAG("MB");

#if (MODBUS_CFG_RTU_EN != 0)
uint32_t  const  MB_TotalRAMSize = sizeof(MB_RTU_Freq)
                                   + sizeof(MB_RTU_TmrCtr)
                                   + sizeof(modbus_channel_table);
#else
uint32_t  const  MB_TotalRAMSize = sizeof(modbus_channel_table);
#endif

uint16_t  const  MB_ChSize       = sizeof(elab_mb_channel_t);

static const osMutexAttr_t mutex_attr_mbm =
{
    "mutex_elog", osMutexRecursive | osMutexPrioInherit, NULL, 0U 
};

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               elab_mb_init()
*
* Description : Handle either Modbus ASCII or Modbus RTU received packets.
*
* Argument(s) : freq       Specifies the Modbus RTU timer frequency (in Hz)
*
* Return(s)   : none.
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void elab_mb_init(uint32_t freq)
{
    uint8_t   ch;
    elab_mb_channel_t   *pch;

#if (MODBUS_CFG_RTU_EN != 0)
    MB_RTU_Freq = freq;                                         /* Save the RTU frequency                             */
#endif

    pch = &modbus_channel_table[0];                                 /* Save Modbus channel number in data structure       */
    for (ch = 0; ch < MODBUS_CFG_MAX_CH; ch++) {                /* Initialize default values                          */
        pch->Ch            = ch;
        pch->NodeAddr      = 1;
        pch->m_or_s   = MODBUS_SLAVE;                      /* Channel defaults to MODBUS_SLAVE mode              */
        pch->Mode          = MODBUS_MODE_ASCII;
        pch->RxBufByteCtr  = 0;
        pch->RxBufPtr      = &pch->RxBuf[0];
        pch->write_en          = MODBUS_WR_EN;
        pch->WrCtr         = 0;
#if (MODBUS_CFG_RTU_EN != 0)
        pch->RTU_TimeoutEn = true;
#endif

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)  && \
    (MODBUS_CFG_FC08_EN  == DEF_ENABLED)
        MBS_StatInit(pch);
#endif
        pch++;
    }

    MB_ChCtr = 0;

    elab_mb_os_init();                                               /* Initialize OS interface functions                  */


#if (MODBUS_CFG_RTU_EN != 0)                         /* MODBUS 'RTU' Initialization                         */
    elab_mb_rtu_timer_init();
#else
    (void)&freq;
#endif
}

/*
*********************************************************************************************************
*                                               elab_mb_exit()
*
* Description : This function is called to terminate all Modbus communications
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  elab_mb_exit (void)
{
#if (MODBUS_CFG_RTU_EN != 0)
    elab_mb_rtu_timer_exit();                                           /* Stop the RTU timer interrupts                      */
#endif

    MB_CommExit();                                              /* Disable all communications                         */

    elab_mb_os_exit();                                               /* Stop RTOS services                                 */
}


/*
*********************************************************************************************************
*                                              elab_mb_config_channel()
*
* Description : This function must be called after calling elab_mb_init() to initialize each of the Modbus
*               channels in your system.
*
* Argument(s) : node_addr     is the Modbus node address that the channel is assigned to.
*
*               master_slave  specifies whether the channel is a MODBUS_MASTER or a MODBUS_SLAVE
*
*               rx_timeout    amount of time Master will wait for a response from the slave.
*
*               modbus_mode   specifies the type of modbus channel.  The choices are:
*                             MODBUS_MODE_ASCII
*                             MODBUS_MODE_RTU
*
*               port_nbr      is the UART port number associated with the channel
*
*               baud          is the desired baud rate
*
*               parity        is the UART's parity setting:
*                             MODBUS_PARITY_NONE
*                             MODBUS_PARITY_ODD
*                             MODBUS_PARITY_EVEN
*
*               bits          UART's number of bits (7 or 8)
*
*               stops         Number of stops bits (1 or 2)
*
*               wr_en         This argument determines whether a Modbus WRITE request will be accepted.
*                             The choices are:
*                             MODBUS_WR_EN
*                             MODBUS_WR_DIS
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

elab_mb_channel_t  *elab_mb_config_channel (uint8_t  node_addr,
                      uint8_t  master_slave,
                      uint32_t  rx_timeout,
                      uint8_t  modbus_mode,
                      uint8_t  port_nbr,
                      uint32_t  baud,
                      uint8_t  bits,
                      uint8_t  parity,
                      uint8_t  stops,
                      uint8_t  wr_en)
{
    elab_mb_channel_t   *pch;
#if (MODBUS_CFG_RTU_EN != 0)
    uint16_t   cnts;
#endif

    if (MB_ChCtr < MODBUS_CFG_MAX_CH) {
        pch = &modbus_channel_table[MB_ChCtr];
        elab_assert(pch != NULL);

        pch->mutex = osMutexNew(&mutex_attr_mbm);
        elab_assert(pch->mutex != NULL);

        pch->RxTimeout = rx_timeout;
        pch->NodeAddr = node_addr;
        elab_mb_set_mode(pch, master_slave, modbus_mode);
        pch->write_en = wr_en;
        pch->PortNbr = port_nbr;
        MB_CommPortCfg(pch, port_nbr, baud, bits, parity, stops);
#if (MODBUS_CFG_RTU_EN != 0)
        if (pch->m_or_s == MODBUS_MASTER) {
            pch->RTU_TimeoutEn = false;
        }

        cnts = ((uint32_t)MB_RTU_Freq * 5L * 10L) / baud;     /* Freq * 5 char * 10 bits/char * 1/baud_rate          */
        if (cnts <= 1) {
            cnts = 5;
        }
        pch->RTU_TimeoutCnts = cnts;
        pch->RTU_TimeoutCtr  = cnts;
#endif
        MB_ChCtr++;

        pch->cb.coil_read = NULL;
        pch->cb.coil_write = NULL;
        pch->cb.di_read = NULL;
        pch->cb.in_reg_read = NULL;
        pch->cb.in_reg_read_fp = NULL;
        pch->cb.holding_reg_read = NULL;
        pch->cb.holding_reg_read_fp = NULL;
        pch->cb.holding_reg_write = NULL;
        pch->cb.holding_reg_write_fp = NULL;
        pch->cb.file_read = NULL;
        pch->cb.file_write = NULL;
        pch->no_ack = false;
        pch->no_ack_timeout_ms = 0;

        return (pch);
    }
    else
    {
        return ((elab_mb_channel_t *)0);
    }
}

void elab_mb_slave_set_cb(elab_mb_channel_t *pch, elab_mb_channel_cb_t *cb)
{
    elab_assert(pch->m_or_s == MODBUS_SLAVE);

    pch->cb.coil_read = cb->coil_read;
    pch->cb.coil_write = cb->coil_write;
    pch->cb.di_read = cb->di_read;
    pch->cb.in_reg_read = cb->in_reg_read;
    pch->cb.in_reg_read_fp = cb->in_reg_read_fp;
    pch->cb.holding_reg_read = cb->holding_reg_read;
    pch->cb.holding_reg_read_fp = cb->holding_reg_read_fp;
    pch->cb.holding_reg_write = cb->holding_reg_write;
    pch->cb.holding_reg_write_fp = cb->holding_reg_write_fp;
    pch->cb.file_read = cb->file_read;
    pch->cb.file_write = cb->file_write;
}

/*
*********************************************************************************************************
*                                         elab_mb_master_set_timeout()
*
* Description : This function is called to change the operating mode of a Modbus channel.
*
* Argument(s) : pch          is a pointer to the Modbus channel to change
*
*               modbus_mode  specifies the type of modbus channel.  The choices are:
*                            MODBUS_MODE_ASCII
*                            MODBUS_MODE_RTU
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void elab_mb_master_set_timeout(elab_mb_channel_t *pch, uint32_t timeout)
{
    if (pch != (elab_mb_channel_t *)0)
    {
        pch->RxTimeout = timeout;
    }
}

/*
*********************************************************************************************************
*                                             elab_mb_set_mode()
*
* Description : This function is called to change the operating mode of a Modbus channel.
*
* Argument(s) : pch          is a pointer to the Modbus channel to change
*
*               modbus_mode  specifies the type of modbus channel.  The choices are:
*                            MODBUS_MODE_ASCII
*                            MODBUS_MODE_RTU
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  elab_mb_set_mode (elab_mb_channel_t  *pch,
                  uint8_t  master_slave,
                  uint8_t  mode)
{
    if (pch != (elab_mb_channel_t *)0) {

        switch (master_slave) {
            case MODBUS_MASTER:
                 pch->m_or_s = MODBUS_MASTER;
                 break;

            case MODBUS_SLAVE:
            default:
                 pch->m_or_s = MODBUS_SLAVE;
                 break;
        }

        switch (mode) {
#if (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
            case MODBUS_MODE_ASCII:
                 pch->Mode = MODBUS_MODE_ASCII;
                 break;
#endif

#if (MODBUS_CFG_RTU_EN != 0)
            case MODBUS_MODE_RTU:
                 pch->Mode = MODBUS_MODE_RTU;
                 break;
#endif

            default:
#if (MODBUS_CFG_RTU_EN != 0)
                 pch->Mode = MODBUS_MODE_RTU;
#else
                 pch->Mode = MODBUS_MODE_ASCII;
#endif
                 break;
        }
    }
}

 
/*
*********************************************************************************************************
*                                              MB_RxByte()
*
* Description : A byte has been received from a serial port.  We just store it in the buffer for processing
*               when a complete packet has been received.
*
* Argument(s) : pch         Is a pointer to the Modbus channel's data structure.
*
*               rx_byte     Is the byte received.
*
* Return(s)   : none.
*
* Caller(s)   : MB_CommRxTxISR_Handler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void MB_RxByte(elab_mb_channel_t *pch, uint8_t rx_byte)
{
    switch (pch->Mode) {
#if (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
        case MODBUS_MODE_ASCII:
             MB_ASCII_RxByte(pch, rx_byte & 0x7F);
             break;
#endif

#if (MODBUS_CFG_RTU_EN != 0)
        case MODBUS_MODE_RTU:
             MB_RTU_RxByte(pch, rx_byte);
             break;
#endif

        default:
             break;
    }
}

 
/*
*********************************************************************************************************
*                                              MB_RxTask()
*
* Description : This function is called when a packet needs to be processed.
*
* Argument(s) : pch         Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : none.
*
* Caller(s)   : MB_OS_RxTask().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void MB_RxTask(elab_mb_channel_t *pch)
{
#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
    if (pch != (elab_mb_channel_t *)0) {
        if (pch->m_or_s == MODBUS_SLAVE) {
            MBS_RxTask(pch);
        }
    }
#endif
}
/*
*********************************************************************************************************
*                                                MB_Tx()
*
* Description : This function is called to start transmitting a packet to a modbus channel.
*
* Argument(s) : pch      Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : none.
*
* Caller(s)   : MB_ASCII_Tx(),
*               MB_RTU_Tx().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_Tx (elab_mb_channel_t  *pch)
{
    pch->TxBufPtr = &pch->TxBuf[0];
    MB_TxByte(pch);
    MB_CommRxIntDis(pch);
    MB_CommTxIntEn(pch);
}

 
/*
*********************************************************************************************************
*                                              MB_TxByte()
*
* Description : This function is called to obtain the next byte to send from the transmit buffer.  When
*               all bytes in the reply have been sent, transmit interrupts are disabled and the receiver
*               is enabled to accept the next Modbus request.
*
* Argument(s) : pch      Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : none.
*
* Caller(s)   : MB_CommRxTxISR_Handler(),
*               MB_Tx().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void MB_TxByte(elab_mb_channel_t  *pch)
{
    uint8_t c;

    if (pch->TxBufByteCtr > 0) {
        pch->TxBufByteCtr--;
        pch->TxCtr++;
        c = *pch->TxBufPtr++;
        MB_CommTx1(pch,                                         /* Write one byte to the serial port                  */
                   c);
#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
        if (pch->m_or_s == MODBUS_MASTER) {
#if (MODBUS_CFG_RTU_EN != 0)
            pch->RTU_TimeoutEn = MODBUS_FALSE;                  /* Disable RTU timeout timer until we start receiving */
#endif
            pch->RxBufByteCtr  = 0;                             /* Flush Rx buffer                                    */
        }
#endif
    } else {                                                    /* If there is nothing to do end transmission         */
        pch->TxBufPtr = &pch->TxBuf[0];                         /* Reset at beginning of buffer                       */
        MB_CommTxIntDis(pch);                                   /* No more data to send, disable Tx interrupts        */
        MB_CommRxIntEn(pch);                                    /* Re-enable the receiver for the next packet         */
    }
}

 
/*
*********************************************************************************************************
*                                           MB_ASCII_RxByte()
*
* Description : A byte has been received from a serial port.  We just store it in the buffer for processing
*               when a complete packet has been received.
*
* Argument(s) : pch         Is a pointer to the Modbus channel's data structure.
*
*               rx_byte     Is the byte received.
*
* Return(s)   : none.
*
* Caller(s)   : MB_RxByte().
*
* Return(s)   : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
void  MB_ASCII_RxByte (elab_mb_channel_t  *pch,
                       uint8_t  rx_byte)
{
    uint8_t   node_addr;
    uint8_t  *phex;


    pch->RxCtr++;                                               /* Increment the number of bytes received             */
    if (rx_byte == ':') {                                       /* Is it the start of frame character?                */
        pch->RxBufPtr     = &pch->RxBuf[0];                     /* Yes, Restart a new frame                           */
        pch->RxBufByteCtr = 0;
    }
    if (pch->RxBufByteCtr < MODBUS_CFG_BUF_SIZE) {              /* No, add received byte to buffer                    */
        *pch->RxBufPtr++  = rx_byte;
        pch->RxBufByteCtr++;                                    /* Increment byte counter to see if we have Rx ...    */
                                                                /* ... activity                                       */
    }
    if (rx_byte == MODBUS_ASCII_END_FRAME_CHAR2) {              /* See if we received a complete ASCII frame          */
        phex      = &pch->RxBuf[1];
        node_addr = MB_ASCII_HexToBin(phex);
        if ((node_addr == pch->NodeAddr) ||                     /* Is the address for us?                             */
            (node_addr == 0)) {                                 /* ... or a 'broadcast'?                              */
            elab_mb_os_rx_signal(pch);                                /* Yes, Let task handle reply                         */
        } else {
            pch->RxBufPtr     = &pch->RxBuf[0];                 /* No,  Wipe out anything, we have to re-synchronize. */
            pch->RxBufByteCtr = 0;
        }
    }
}
#endif

 
/*
*********************************************************************************************************
*                                             MB_ASCII_Rx()
*
* Description : Parses and converts an ASCII style message into a Modbus frame.  A check is performed
*               to verify that the Modbus packet is valid.
*
* Argument(s) : pch         Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true        If all checks pass.
*               false       If any checks fail.
*
* Caller(s)   : MBM_RxReply().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
bool  MB_ASCII_Rx (elab_mb_channel_t  *pch)
{
    uint8_t  *pmsg;
    uint8_t  *prx_data;
    uint16_t   rx_size;


    pmsg      = &pch->RxBuf[0];
    rx_size   =  pch->RxBufByteCtr;
    prx_data  = &pch->RxFrameData[0];
    if ((rx_size & 0x01)                                     &&        /* Message should have an ODD nbr of bytes.        */
        (rx_size            > MODBUS_ASCII_MIN_MSG_SIZE)     &&        /* Check if message is long enough                 */
        (pmsg[0]           == MODBUS_ASCII_START_FRAME_CHAR) &&        /* Check the first char.                           */
        (pmsg[rx_size - 2] == MODBUS_ASCII_END_FRAME_CHAR1)  &&        /* Check the last two.                             */
        (pmsg[rx_size - 1] == MODBUS_ASCII_END_FRAME_CHAR2)) {
        rx_size               -= 3;                                    /* Take away for the ':', CR, and LF               */
        pmsg++;                                                        /* Point past the ':' to the address.              */
        pch->RxFrameNDataBytes = 0;                                    /* Get the data from the message                   */
        while (rx_size > 2) {
            *prx_data++  = MB_ASCII_HexToBin(pmsg);
            pmsg        += 2;
            rx_size     -= 2;
            pch->RxFrameNDataBytes++;                                  /* Increment the number of Modbus packets received */
        }
        pch->RxFrameNDataBytes -= 2;                                   /* Subtract the Address and function code          */
        pch->RxFrameCRC         = (uint16_t)MB_ASCII_HexToBin(pmsg); /* Extract the message's LRC                       */
        return (true);
    } else {
        return (false);
    }
}
#endif

 
/*
*********************************************************************************************************
*                                             MB_ASCII_Tx()
*
* Description : The format of the message is ASCII.  The actual information is taken from the given
*               MODBUS frame.
*
* Argument(s) : pch      Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : none.
*
* Caller(s)   : MBM_TxCmd(),
*               MBS_ASCII_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
void  MB_ASCII_Tx (elab_mb_channel_t  *pch)
{
    uint8_t  *ptx_data;
    uint8_t  *pbuf;
    uint16_t   i;
    uint16_t   tx_bytes;
    uint8_t   lrc;


    ptx_data = &pch->TxFrameData[0];
    pbuf     = &pch->TxBuf[0];
    *pbuf++  = MODBUS_ASCII_START_FRAME_CHAR;                   /* Place the start-of-frame character into output buffer  */
    pbuf     = MB_ASCII_BinToHex(*ptx_data++,
                                 pbuf);
    pbuf     = MB_ASCII_BinToHex(*ptx_data++,
                                 pbuf);
    tx_bytes = 5;
    i        = (uint8_t)pch->TxFrameNDataBytes;              /* Transmit the actual data                               */
    while (i > 0) {
        pbuf      = MB_ASCII_BinToHex(*ptx_data++,
                                      pbuf);
        tx_bytes += 2;
        i--;
    }
    lrc               = MB_ASCII_TxCalcLRC(pch,                 /* Compute outbound packet LRC                            */
                                           tx_bytes);
    pbuf              = MB_ASCII_BinToHex(lrc,                  /* Add the LRC checksum in the packet                     */
                                          pbuf);
    *pbuf++           = MODBUS_ASCII_END_FRAME_CHAR1;           /* Add 1st end-of-frame character (0x0D) to output buffer */
    *pbuf++           = MODBUS_ASCII_END_FRAME_CHAR2;           /* Add 2nd end-of-frame character (0x0A) to output buffer */
    tx_bytes         += 4;
    pch->TxFrameCRC   = (uint16_t)lrc;                        /* Save the computed LRC into the channel                 */
    pch->TxBufByteCtr = tx_bytes;                               /* Update the total number of bytes to send               */
    MB_Tx(pch);                                                 /* Send it out the communication driver.                  */
}
#endif

 
/*
*********************************************************************************************************
*                                            MB_RTU_RxByte()
*
* Description : A byte has been received from a serial port.  We just store it in the buffer for processing
*               when a complete packet has been received.
*
* Argument(s) : pch         Is a pointer to the Modbus channel's data structure.
*
*               rx_byte     Is the byte received.
*
* Return(s)   : none.
*
* Caller(s)   : MB_RxByte().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_RTU_EN != 0)
void  MB_RTU_RxByte (elab_mb_channel_t  *pch,
                     uint8_t  rx_byte)
{
    MB_RTU_TmrReset(pch);                                       /* Reset the timeout timer on a new character             */
#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
    if (pch->m_or_s == MODBUS_MASTER) {
        pch->RTU_TimeoutEn = MODBUS_TRUE;
    }
#endif
    if (pch->RxBufByteCtr < MODBUS_CFG_BUF_SIZE) {              /* No, add received byte to buffer                        */
        pch->RxCtr++;                                           /* Increment the number of bytes received                 */
        *pch->RxBufPtr++ = rx_byte;
        pch->RxBufByteCtr++;                                    /* Increment byte counter to see if we have Rx activity   */
    }
}
#endif

 
/*
*********************************************************************************************************
*                                              MB_RTU_Rx()
*
* Description : Parses a Modbus RTU packet and processes the request if the packet is valid.
*
* Argument(s) : pch         Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true    If all checks pass.
*               false   If any checks fail.
*
* Caller(s)   : MBM_RxReply(),
*               MBS_RTU_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_RTU_EN != 0)
bool  MB_RTU_Rx (elab_mb_channel_t  *pch)
{
    uint8_t  *prx_data;
    uint8_t  *pmsg;
    uint16_t   rx_size;
    uint16_t   crc;


    pmsg    = &pch->RxBuf[0];
    rx_size =  pch->RxBufByteCtr;
    if (rx_size >= MODBUS_RTU_MIN_MSG_SIZE) {         /* Is the message long enough?                        */
        if (rx_size <= MODBUS_CFG_BUF_SIZE) {
            prx_data    = &pch->RxFrameData[0];
            *prx_data++ = *pmsg++;                    /* Transfer the node address                          */
            rx_size--;

            *prx_data++ = *pmsg++;                    /* Transfer the function code                         */
            rx_size--;

            pch->RxFrameNDataBytes = 0;               /* Transfer the data                                  */
            while (rx_size > 2) {
                *prx_data++ = *pmsg++;
                pch->RxFrameNDataBytes++;
                rx_size--;
            }

            crc              = (uint16_t)*pmsg++;   /* Transfer the CRC over.  It's LSB first, then MSB.  */
            crc             += (uint16_t)*pmsg << 8;
            pch->RxFrameCRC  = crc;
            return (true);
        } else {
            return (false);
        }
    } else {
        return (false);
    }
}
#endif

 
/*
*********************************************************************************************************
*                                              MB_RTU_Tx()
*
* Description : A MODBUS message is formed into a buffer and sent to the appropriate communication port.
*               The actual reply is taken from the given MODBUS Frame.
*
* Argument(s) : pch      Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : none.
*
* Caller(s)   : MBM_TxCmd(),
*               MBS_RTU_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_RTU_EN != 0)
void  MB_RTU_Tx (elab_mb_channel_t  *pch)
{
    uint8_t  *ptx_data;
    uint8_t  *pbuf;
    uint8_t   i;
    uint16_t   tx_bytes;
    uint16_t   crc;


    tx_bytes  = 0;
    pbuf      = &pch->TxBuf[0];                                    /* Point to the beginning of the output buffer.             */
    ptx_data  = &(pch->TxFrameData[0]);
    i         = (uint8_t)pch->TxFrameNDataBytes + 2;            /* Include the actual data in the buffer                    */
    while (i > 0) {
        *pbuf++ = *ptx_data++;
        tx_bytes++;
        i--;
    }
    crc               = MB_RTU_TxCalcCRC(pch);
    *pbuf++           = (uint8_t)(crc & 0x00FF);                /* Add in the CRC checksum.  Low byte first!                */
    *pbuf             = (uint8_t)(crc >> 8);
    tx_bytes         += 2;
    pch->TxFrameCRC   = crc;                                       /* Save the calculated CRC in the channel                   */
    pch->TxBufByteCtr = tx_bytes;

    MB_Tx(pch);                                                    /* Send it out the communication driver.                    */
}
#endif

 
/*
*********************************************************************************************************
*                                           MB_RTU_TmrReset()
*
* Description : This function is called when a byte a received and thus, we reset the RTU timeout timer value
*               indicating that we are not done receiving a complete RTU packet.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : MB_RTU_TmrResetAll().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_RTU_EN != 0)
void  MB_RTU_TmrReset (elab_mb_channel_t  *pch)
{
    pch->RTU_TimeoutCtr = pch->RTU_TimeoutCnts;
}
#endif

 
/*
*********************************************************************************************************
*                                           MB_RTU_TmrResetAll()
*
* Description : This function is used to reset all the RTU timers for all Modbus channels.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : elab_mb_rtu_timer_init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_RTU_EN != 0)
void  MB_RTU_TmrResetAll (void)
{
    uint8_t ch;
    elab_mb_channel_t *pch;

    pch = &modbus_channel_table[0];
    for (ch = 0; ch < MODBUS_CFG_MAX_CH; ch++) {
        if (pch->Mode == MODBUS_MODE_RTU) {
            MB_RTU_TmrReset(pch);
        }
        pch++;
    }
}
#endif

 
/*
*********************************************************************************************************
*                                           MB_RTU_TmrUpdate()
*
* Description : This function is called when the application supplied RTU framing timer expires.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : MB_RTU_TmrISR_Handler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_RTU_EN != 0)
void  MB_RTU_TmrUpdate (void)
{
    uint8_t   ch;
    elab_mb_channel_t   *pch;

    pch = &modbus_channel_table[0];
    for (ch = 0; ch < MODBUS_CFG_MAX_CH; ch++) {
        if (pch->Mode == MODBUS_MODE_RTU) {
            if (pch->RTU_TimeoutEn == true) {
                if (pch->RTU_TimeoutCtr > 0) {
                    pch->RTU_TimeoutCtr--;
                    if (pch->RTU_TimeoutCtr == 0) {
#if (MODBUS_CFG_RTU_EN != 0)
                        if (pch->m_or_s == MODBUS_MASTER) {
                            pch->RTU_TimeoutEn = false;
                        }
#endif
                        elab_mb_os_rx_signal(pch);          /* RTU Timer expired for this Modbus channel         */
                    }
                }
            } else {
                pch->RTU_TimeoutCtr = pch->RTU_TimeoutCnts;
            }
        }
        pch++;
    }
}
#endif
