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
*                                     uC/MODBUS SLAVE COMMAND PROCESSOR
*
* Filename : mbs_core.c
* Version  : V2.14.00
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define   MBS_MODULE

#include "elab/common/elab_assert.h"
#include "mb.h"

ELAB_TAG("ModbusSlaveCore");

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)

#define  MBS_RX_DATA_START     (((uint16_t)pch->RxFrameData[2] << 8) + (uint16_t)pch->RxFrameData[3])
#define  MBS_RX_DATA_START_H    (pch->RxFrameData[2])
#define  MBS_RX_DATA_START_L    (pch->RxFrameData[3])

#define  MBS_RX_DATA_POINTS    (((uint16_t)pch->RxFrameData[4] << 8) + (uint16_t)pch->RxFrameData[5])
#define  MBS_RX_DATA_POINTS_H   (pch->RxFrameData[4])
#define  MBS_RX_DATA_POINTS_L   (pch->RxFrameData[5])

#define  MBS_RX_DATA_BYTES      (pch->RxFrameData[6])

#define  MBS_RX_DATA_COIL      (((uint16_t)pch->RxFrameData[4] << 8) + (uint16_t)pch->RxFrameData[5])
#define  MBS_RX_DATA_COIL_H     (pch->RxFrameData[4])
#define  MBS_RX_DATA_COIL_L     (pch->RxFrameData[5])

#define  MBS_RX_DATA_REG       (((uint16_t)pch->RxFrameData[4] << 8) + (uint16_t)pch->RxFrameData[5])
#define  MBS_RX_DATA_REG_H      (pch->RxFrameData[4])
#define  MBS_RX_DATA_REG_L      (pch->RxFrameData[5])

#define  MBS_RX_DIAG_CODE      (((uint16_t)pch->RxFrameData[2] << 8) + (uint16_t)pch->RxFrameData[3])
#define  MBS_RX_DIAG_CODE_H     (pch->RxFrameData[2])
#define  MBS_RX_DIAG_CODE_L     (pch->RxFrameData[3])
#define  MBS_RX_DIAG_DATA      (((uint16_t)pch->RxFrameData[4] << 8) + (uint16_t)pch->RxFrameData[5])
#define  MBS_RX_DIAG_DATA_H     (pch->RxFrameData[4])
#define  MBS_RX_DIAG_DATA_L     (pch->RxFrameData[5])

#define  MBS_RX_FRAME          (&pch->RxFrame)
#define  MBS_RX_FRAME_ADDR      (pch->RxFrameData[0])
#define  MBS_RX_FRAME_FC        (pch->RxFrameData[1])
#define  MBS_RX_FRAME_DATA      (pch->RxFrameData[2])
#define  MBS_RX_FRAME_NBYTES    (pch->RxFrameNDataBytes)


#define  MBS_TX_DATA_START_H    (pch->TxFrameData[2])
#define  MBS_TX_DATA_START_L    (pch->TxFrameData[3])

#define  MBS_TX_DATA_POINTS_H   (pch->TxFrameData[4])
#define  MBS_TX_DATA_POINTS_L   (pch->TxFrameData[5])

#define  MBS_TX_DATA_COIL_H     (pch->TxFrameData[4])
#define  MBS_TX_DATA_COIL_L     (pch->TxFrameData[5])

#define  MBS_TX_DATA_REG_H      (pch->TxFrameData[4])
#define  MBS_TX_DATA_REG_L      (pch->TxFrameData[5])

#define  MBS_TX_DIAG_CODE_H     (pch->TxFrameData[2])
#define  MBS_TX_DIAG_CODE_L     (pch->TxFrameData[3])
#define  MBS_TX_DIAG_DATA_H     (pch->TxFrameData[4])
#define  MBS_TX_DIAG_DATA_L     (pch->TxFrameData[5])


#define  MBS_TX_FRAME           (&pch->TxFrame)
#define  MBS_TX_FRAME_ADDR      (pch->TxFrameData[0])
#define  MBS_TX_FRAME_FC        (pch->TxFrameData[1])
#define  MBS_TX_FRAME_DATA      (pch->TxFrameData[2])
#define  MBS_TX_FRAME_NBYTES    (pch->TxFrameNDataBytes)


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)

static  void         MBS_ErrRespSet               (elab_mb_channel_t   *pch,
                                                   uint8_t   errcode);

#if     (MODBUS_CFG_FC01_EN == DEF_ENABLED)
static  bool  MBS_FC01_CoilRd              (elab_mb_channel_t   *pch);
#endif

#if     (MODBUS_CFG_FC02_EN == DEF_ENABLED)
static  bool  MBS_FC02_DIRd                (elab_mb_channel_t   *pch);
#endif

#if     (MODBUS_CFG_FC03_EN == DEF_ENABLED)
static  bool  MBS_FC03_HoldingRegRd        (elab_mb_channel_t   *pch);
#endif

#if     (MODBUS_CFG_FC04_EN == DEF_ENABLED)
static  bool  MBS_FC04_InRegRd             (elab_mb_channel_t   *pch);
#endif

#if     (MODBUS_CFG_FC05_EN == DEF_ENABLED)
static  bool  MBS_FC05_CoilWr              (elab_mb_channel_t   *pch);
#endif

#if     (MODBUS_CFG_FC06_EN == DEF_ENABLED)
static  bool  MBS_FC06_HoldingRegWr        (elab_mb_channel_t   *pch);
#endif

#if     (MODBUS_CFG_FC08_EN == DEF_ENABLED)
static  bool  MBS_FC08_Loopback            (elab_mb_channel_t   *pch);
#endif

#if     (MODBUS_CFG_FC15_EN == DEF_ENABLED)
static  bool  MBS_FC15_CoilWrMultiple      (elab_mb_channel_t   *pch);
#endif

#if     (MODBUS_CFG_FC16_EN == DEF_ENABLED)
static  bool  MBS_FC16_HoldingRegWrMultiple(elab_mb_channel_t   *pch);
#endif

#if     (MODBUS_CFG_FC20_EN == DEF_ENABLED)
static  bool  MBS_FC20_FileRd              (elab_mb_channel_t   *pch);
#endif

#if     (MODBUS_CFG_FC21_EN == DEF_ENABLED)
static  bool  MBS_FC21_FileWr              (elab_mb_channel_t   *pch);
#endif

#if     (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
static  void         MBS_ASCII_Task               (elab_mb_channel_t   *pch);
#endif

#if     (MODBUS_CFG_RTU_EN   == DEF_ENABLED)
static  void         MBS_RTU_Task                 (elab_mb_channel_t   *pch);
#endif

#endif


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         MBS_ErrRespSet()
*
* Description : This function sets the indicated error response code into the response frame.  Then the
*               routine is called to calculate the error check value.
*
* Argument(s) : pch         Is a pointer to the Modbus channel's data structure.
*
*               errcode     An unsigned byte value containing the error code that is to be placed in the
*                           response frame.
*
* Return(s)   : none.
*
* Caller(s)   : MBS_FCxx_Handler(),
*               Modbus Slave functions.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
static void MBS_ErrRespSet (elab_mb_channel_t  *pch,
                              uint8_t  err_code)
{
#if (MODBUS_CFG_FC08_EN == DEF_ENABLED)
    pch->StatExceptCtr++;
#endif
    MBS_TX_FRAME_ADDR   = MBS_RX_FRAME_ADDR;
    MBS_TX_FRAME_FC     = MBS_RX_FRAME_FC | 0x80;                /* Set the high order bit of the function code.             */
    MBS_TX_FRAME_DATA   = err_code;                              /* Set the high order bit of the function code.             */
    MBS_TX_FRAME_NBYTES = 1;                                     /* Nbr of data bytes in exception response is 1.            */
}
#endif

/*
*********************************************************************************************************
*                                           MBS_FCxx_Handler()
*
* Description : This is the main processing function for MODBUS commands.  The message integrity is
*               verified, and if valid, the function requested is processed.  Unimplemented functions
*               will generate an Illegal Function Exception Response code (01).
*
* Argument(s) : pch      Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : none.
*
* Caller(s)   : MBS_ASCII_Task(),
*               MBS_RTU_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
bool  MBS_FCxx_Handler (elab_mb_channel_t  *pch)
{
    bool  send_reply;


    send_reply = false;
    if ((MBS_RX_FRAME_ADDR == pch->NodeAddr) ||              /* Proper node address? (i.e. Is this message for us?)      */
        (MBS_RX_FRAME_ADDR == 0)) {                          /* ... or a 'broadcast' address?                            */
#if (MODBUS_CFG_FC08_EN == DEF_ENABLED)
        pch->StatSlaveMsgCtr++;
#endif
        switch (MBS_RX_FRAME_FC) {                           /* Handle the function requested in the frame.              */
#if (MODBUS_CFG_FC01_EN == DEF_ENABLED)
            case MODBUS_FC01_COIL_RD:
                 send_reply = MBS_FC01_CoilRd(pch);
                 break;
#endif

#if (MODBUS_CFG_FC02_EN == DEF_ENABLED)
            case MODBUS_FC02_DI_RD:
                 send_reply = MBS_FC02_DIRd(pch);
                 break;
#endif

#if (MODBUS_CFG_FC03_EN == DEF_ENABLED)
            case MODBUS_FC03_HOLDING_REG_RD:                 /* Process read output registers command.                   */
                 send_reply = MBS_FC03_HoldingRegRd(pch);
                 break;
#endif

#if (MODBUS_CFG_FC04_EN == DEF_ENABLED)
            case MODBUS_FC04_IN_REG_RD:
                 send_reply = MBS_FC04_InRegRd(pch);
                 break;
#endif

#if (MODBUS_CFG_FC05_EN == DEF_ENABLED)
            case MODBUS_FC05_COIL_WR:
                 if (pch->write_en == true) {
                     send_reply  = MBS_FC05_CoilWr(pch);
                 } else {
                     send_reply  = false;
                 }
                 break;
#endif

#if (MODBUS_CFG_FC06_EN == DEF_ENABLED)
            case MODBUS_FC06_HOLDING_REG_WR:
                 if (pch->write_en == true) {
                     send_reply  = MBS_FC06_HoldingRegWr(pch);
                 } else {
                     send_reply  = false;
                 }
                 break;
#endif

#if (MODBUS_CFG_FC08_EN == DEF_ENABLED)
            case MODBUS_FC08_LOOPBACK:
                 send_reply = MBS_FC08_Loopback(pch);        /* Process loopback command.                                */
                 break;
#endif

#if (MODBUS_CFG_FC15_EN == DEF_ENABLED)
            case MODBUS_FC15_COIL_WR_MULTIPLE:
                 if (pch->write_en == true) {
                     send_reply  = MBS_FC15_CoilWrMultiple(pch);
                 } else {
                     send_reply  = false;
                 }
                 break;
#endif

#if (MODBUS_CFG_FC16_EN == DEF_ENABLED)
            case MODBUS_FC16_HOLDING_REG_WR_MULTIPLE:
                 if (pch->write_en == true) {
                     send_reply  = MBS_FC16_HoldingRegWrMultiple(pch);
                 } else {
                     send_reply  = false;
                 }
                 break;
#endif

#if (MODBUS_CFG_FC20_EN == DEF_ENABLED)
            case MODBUS_FC20_FILE_RD:
                 send_reply  = MBS_FC20_FileRd(pch);
                 break;
#endif

#if (MODBUS_CFG_FC21_EN == DEF_ENABLED)
            case MODBUS_FC21_FILE_WR:
                 if (pch->write_en == true) {
                     send_reply = MBS_FC21_FileWr(pch);
                 } else {
                     send_reply = false;
                 }
                 break;
#endif

            default:                                         /* Function code not implemented, set error response.       */
                pch->Err = MODBUS_ERR_ILLEGAL_FC;
                MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_FC);
                send_reply = true;
                break;
        }
    }

    /* If the command received a 'broadcast', */
    if (MBS_RX_FRAME_ADDR == 0)
    {
        /* Yes, don't reply. */
        return (false);
    }
    else
    {
        /* No,  reply according to the outcome of the command. */
        return (send_reply);
    }
}
#endif

/*
*********************************************************************************************************
*                                           MBS_FC01_CoilRd()
*
* Description : Responds to a request to read the status of any number of coils.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : MBS_FCxx_Handler().
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x01
*                  <start address HI>             0x00
*                  <start address LO>             0x13
*                  <# coils HI>                   0x00
*                  <# coils LO>                   0x25
*                  <Error Check (LRC or CRC)>     0x??
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x01
*                  <byte count>                   0x05
*                  <Data Coils>                   0xCD  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Data Coils>                   0x6B  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Data Coils>                   0xB2  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Data Coils>                   0x0E  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Data Coils>                   0x1B  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Error Check (LRC or CRC)>     0x??
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC01_EN  == DEF_ENABLED)
static  bool  MBS_FC01_CoilRd (elab_mb_channel_t  *pch)
{
    uint8_t   *presp;
    uint16_t    coil;
    bool   coil_val;
    uint16_t    err;
    uint16_t    nbr_coils;
    uint16_t    nbr_bytes;
    uint8_t    bit_mask;
    uint16_t    ix;


    if (pch->RxFrameNDataBytes != 4) {                           /* 4 data bytes for this message.                           */
        return (false);                                      /* Tell caller that we DON'T need to send a response        */
    }
    coil      = MBS_RX_DATA_START;                               /* Get the starting address of the desired coils            */
    nbr_coils = MBS_RX_DATA_POINTS;                              /* Find out how many coils                                  */
    if (nbr_coils == 0 || nbr_coils > 2000) {                    /* Make sure we don't exceed the allowed limit per request  */
        pch->Err = MODBUS_ERR_FC01_01;
        MBS_ErrRespSet(pch,
                       MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (true);                                       /* Tell caller that we need to send a response              */
    }
    nbr_bytes              = ((nbr_coils - 1) / 8) + 1;          /* Find #bytes needed for response.                         */
    pch->TxFrameNDataBytes = nbr_bytes + 1;                      /* Number of data bytes + byte count.                       */
    presp                  = &pch->TxFrameData[0];               /* Clear bytes in response                                  */
    for (ix = 0; ix < (nbr_bytes + 3); ix++) {
        *presp++ = 0x00;
    }
    bit_mask = 0x01;                                             /* Start with bit 0 in response byte data mask.             */
    ix       =    0;                                             /* Initialize loop counter.                                 */
    presp    = &pch->TxFrameData[0];                             /* Reset the pointer to the start of the response           */
    *presp++ = MBS_RX_FRAME_ADDR;                                /* Prepare response packet                                  */
    *presp++ = MBS_RX_FRAME_FC;
    *presp++ = (uint8_t)nbr_bytes;                            /* Set number of data bytes in response message.            */
    while (ix < nbr_coils) {                                     /* Loop through each COIL requested.                        */
        /* Get the current value of the coil. */
        elab_assert(pch->cb.coil_read != NULL);
        coil_val = pch->cb.coil_read(coil, &err);
        switch (err) {
            case MODBUS_ERR_NONE:
                if (coil_val == MODBUS_COIL_ON) {               /* Only set data response bit if COIL is on.                */
                    *presp |= bit_mask;
                }
                coil++;
                ix++;                                           /* Increment COIL counter.                                  */
                if ((ix % 8) == 0) {                            /* Determine if 8 data bits have been filled.               */
                    bit_mask   = 0x01;                          /* Reset the data mask.                                     */
                    presp++;                                    /* Increment data frame index.                              */
                } else {                                        /* Still in same data byte, so                              */
                    bit_mask <<= 1;                             /* Shift the data mask to the next higher bit position.     */
                }
                break;

            case MODBUS_ERR_RANGE:
            default:
                pch->Err = MODBUS_ERR_FC01_02;
                MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
                return (true);                              /* Tell caller that we need to send a response. */
        }
    }
    pch->Err = MODBUS_ERR_NONE;
    return (true);                                           /* Tell caller that we need to send a response              */
}
#endif
#endif

/*
*********************************************************************************************************
*                                            MBS_FC02_DIRd()
*
* Description : Responds to a request to read the status of any number of Discrete Inputs (DIs).
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : MBS_FCxx_Handler().
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x02
*                  <start address HI>             0x00
*                  <start address LO>             0xC4
*                  <# input statuses HI>          0x00
*                  <# input statuses LO>          0x16
*                  <Error Check (LRC or CRC)>     0x??
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x02
*                  <byte count>                   0x03
*                  <Data Inputs>                  0xAC  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Data Inputs>                  0xDB  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Data Inputs>                  0x35  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Error Check (LRC or CRC)>     0x??
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC02_EN  == DEF_ENABLED)
static  bool  MBS_FC02_DIRd (elab_mb_channel_t  *pch)
{
    uint8_t   *presp;
    uint16_t    di;
    bool   di_val;
    uint16_t    err;
    uint16_t    nbr_di;
    uint16_t    nbr_bytes;
    uint8_t    bit_mask;
    uint16_t    ix;


    if (pch->RxFrameNDataBytes != 4) {                           /* 4 data bytes for this message.                           */
        return (false);                                      /* Tell caller that we DON'T need to send a response        */
    }
    di     = MBS_RX_DATA_START;                                  /* Get the starting address of the desired DIs              */
    nbr_di = MBS_RX_DATA_POINTS;                                 /* Find out how many DIs                                    */
    if (nbr_di == 0 || nbr_di > 2000) {                          /* Make sure we don't exceed the allowed limit per request  */
        pch->Err = MODBUS_ERR_FC02_01;
        MBS_ErrRespSet(pch,
                       MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (true);                                       /* Tell caller that we need to send a response              */
    }
    nbr_bytes             = ((nbr_di - 1) / 8) + 1;              /* Find #bytes needed for response.                         */
    pch->TxFrameNDataBytes = nbr_bytes + 1;                      /* Number of data bytes + byte count.                       */
    presp                 = &pch->TxFrameData[0];                /* Clear bytes in response                                  */
    for (ix = 0; ix < (nbr_bytes + 3); ix++) {
        *presp++ = 0x00;
    }
    bit_mask = 0x01;                                             /* Start with bit 0 in response byte data mask.             */
    ix       =    0;                                             /* Initialize loop counter.                                 */
    presp    = &pch->TxFrameData[0];                             /* Reset the pointer to the start of the response           */
    *presp++ =  MBS_RX_FRAME_ADDR;                               /* Prepare response packet                                  */
    *presp++ =  MBS_RX_FRAME_FC;
    *presp++ = (uint8_t)nbr_bytes;                            /* Set number of data bytes in response message.            */
    while (ix < nbr_di) {                                        /* Loop through each DI requested.                          */
        /* Get the current value of the DI. */
        elab_assert(pch->cb.di_read != NULL);
        di_val = pch->cb.di_read(di, &err);
        switch (err) {
            case MODBUS_ERR_NONE:
                if (di_val == MODBUS_COIL_ON) {                 /* Only set data response bit if DI is on.                  */
                    *presp |= bit_mask;
                }
                di++;
                ix++;                                           /* Increment DI counter.                                    */
                if ((ix % 8) == 0) {                            /* Determine if 8 data bits have been filled.               */
                    bit_mask   = 0x01;                          /* Reset the data mask.                                     */
                    presp++;                                    /* Increment data frame index.                              */
                } else {                                        /* Still in same data byte, so                              */
                    bit_mask <<= 1;                             /* Shift the data mask to the next higher bit position.     */
                }
                break;

            case MODBUS_ERR_RANGE:
            default:
                 pch->Err = MODBUS_ERR_FC02_02;
                 MBS_ErrRespSet(pch,
                                MODBUS_ERR_ILLEGAL_DATA_ADDR);
                 return (true);                              /* Tell caller that we need to send a response              */
        }
    }
    pch->Err = MODBUS_ERR_NONE;
    return (true);                                           /* Tell caller that we need to send a response              */
}
#endif
#endif

/*
*********************************************************************************************************
*                                        MBS_FC03_HoldingRegRd()
*
* Description : Obtains the contents of the specified holding registers.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : MBS_FCxx_Handler().
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x03
*                  <start address HI>             0x00
*                  <start address LO>             0x6B
*                  <# registers HI>               0x00
*                  <# registers LO>               0x03
*                  <Error Check (LRC or CRC)>     0x??
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x03
*                  <byte count>                   0x06
*                  <Data HI register>             0x02
*                  <Data LO register>             0x2B
*                  <Data HI register>             0x00
*                  <Data LO register>             0x00
*                  <Data HI register>             0x00
*                  <Data LO register>             0x64
*                  <Error Check (LRC or CRC)>     0x??
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC03_EN  == DEF_ENABLED)
static  bool  MBS_FC03_HoldingRegRd (elab_mb_channel_t  *pch)
{
    uint8_t  *presp;
    uint16_t err;
    uint16_t   reg;
    uint16_t   nbr_regs;
    uint16_t   nbr_bytes;
    uint16_t   reg_val_16;
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
    uint8_t   ix;
    float     reg_val_fp;
    uint8_t  *pfp;
#endif


    if (pch->RxFrameNDataBytes != 4) {                           /* Nbr of data bytes must be 4.                             */
        return (false);                                      /* Tell caller that we DON'T need to send a response        */
    }
    reg       = MBS_RX_DATA_START;
    nbr_regs  = MBS_RX_DATA_POINTS;
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
    if (reg < MODBUS_CFG_FP_START_IX) {                          /* See if we want integer registers                         */
        if (nbr_regs == 0 || nbr_regs > 125) {                   /* Make sure we don't exceed the allowed limit per request  */
            pch->Err = MODBUS_ERR_FC03_03;
            MBS_ErrRespSet(pch,
                           MODBUS_ERR_ILLEGAL_DATA_QTY);
            return (true);                                   /* Tell caller that we need to send a response              */
        }
        nbr_bytes = (uint8_t)(nbr_regs * sizeof(uint16_t)); /* Find #bytes needed for response.                         */
    } else {
        if (nbr_regs == 0 || nbr_regs > 62) {                    /* Make sure we don't exceed the allowed limit per request  */
            pch->Err = MODBUS_ERR_FC03_04;
            MBS_ErrRespSet(pch,
                           MODBUS_ERR_ILLEGAL_DATA_QTY);
            return (true);                                   /* Tell caller that we need to send a response              */
        }
        nbr_bytes = (uint8_t)(nbr_regs * sizeof(float));   /* Find #bytes needed for response.                         */
    }
#else
    if (nbr_regs == 0 || nbr_regs > 125) {                       /* Make sure we don't exceed the allowed limit per request  */
        pch->Err = MODBUS_ERR_FC03_03;
        MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (true);                                       /* Tell caller that we need to send a response              */
    }
    nbr_bytes = (uint8_t)(nbr_regs * sizeof(uint16_t));     /* Find #bytes needed for response.                         */
#endif
    pch->TxFrameNDataBytes = nbr_bytes + 1;                      /* Number of data bytes + byte count.                       */
    presp                 = &pch->TxFrameData[0];                /* Reset the pointer to the start of the response           */
    *presp++              =  MBS_RX_FRAME_ADDR;
    *presp++              =  MBS_RX_FRAME_FC;
    *presp++              = (uint8_t)nbr_bytes;               /* Set number of data bytes in response message             */

    /* Loop through each register requested. */
    while (nbr_regs > 0)
    {
        /* See if we want an integer register. */
        if (reg < MODBUS_CFG_FP_START_IX)
        {
            /* Yes, get its value. */
            elab_assert(pch->cb.holding_reg_read != NULL);
            reg_val_16 = pch->cb.holding_reg_read(reg, &err);
            switch (err)
            {
                case MODBUS_ERR_NONE:
                    *presp++ = (uint8_t)((reg_val_16 >> 8) & 0x00FF); /* MSB */
                    *presp++ = (uint8_t)(reg_val_16 & 0x00FF);        /* LSB */
                    break;

                case MODBUS_ERR_RANGE:
                default:
                    pch->Err = MODBUS_ERR_FC03_01;
                    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
                    return (true);
            }
        }
        else
        {
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
            /* No, get the value of the FP register. */
            elab_assert(pch->cb.holding_reg_read_fp != NULL);
            reg_val_fp = pch->cb.holding_reg_read_fp(reg, &err);
            switch (err)
            {
                case MODBUS_ERR_NONE:
                    /* Point to the FP register. */
                    pfp = (uint8_t *)&reg_val_fp;            
#if (CPU_CFG_ENDIAN_TYPE == CPU_ENDIAN_TYPE_BIG)
                    for (ix = 0; ix < sizeof(float); ix++) { /* Copy value to response buffer                            */
                        *presp++ = *pfp++;
                    }
#else
                    pfp += sizeof(float) - 1;
                    for (ix = 0; ix < sizeof(float); ix++) {
                        *presp++ = *pfp--;
                    }
#endif
                    break;

                case MODBUS_ERR_RANGE:
                default:
                    pch->Err = MODBUS_ERR_FC03_02;
                    MBS_ErrRespSet(pch,
                                MODBUS_ERR_ILLEGAL_DATA_ADDR);
                    return (true);
            }
#endif
        }

        /* Increment current register number. */
        reg ++;
        nbr_regs --;
    }
    pch->Err = MODBUS_ERR_NONE;

    /* Tell caller that we need to send a response. */
    return (true);
}
#endif
#endif

/*
*********************************************************************************************************
*                                          MBS_FC04_InRegRd()
*
* Description : Obtains the contents of the specified input registers.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : MBS_FCxx_Handler().
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x04
*                  <start address HI>             0x00
*                  <start address LO>             0x08
*                  <# registers HI>               0x00
*                  <# registers LO>               0x01
*                  <Error Check (LRC or CRC)>     0x??
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x04
*                  <byte count>                   0x02
*                  <Data HI register value>       0x00
*                  <Data LO register value>       0x0A
*                  <Error Check (LRC or CRC)>     0x??
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC04_EN  == DEF_ENABLED)
static  bool  MBS_FC04_InRegRd (elab_mb_channel_t  *pch)
{
    uint8_t  *presp;
    uint16_t err;
    uint16_t   reg;
    uint16_t   nbr_regs;
    uint16_t   nbr_bytes;
    uint16_t   reg_val_16;
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
    uint8_t   ix;
    float     reg_val_fp;
    uint8_t  *pfp;
#endif


    if (pch->RxFrameNDataBytes != 4) {                           /* Nbr of data bytes must be 4.                             */
        return (false);                                      /* Tell caller that we DON'T need to send a response        */
    }
    reg       = MBS_RX_DATA_START;
    nbr_regs  = MBS_RX_DATA_POINTS;
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
    if (reg < MODBUS_CFG_FP_START_IX) {                          /* See if we want integer registers                         */
        if (nbr_regs == 0 || nbr_regs > 125) {                   /* Make sure we don't exceed the allowed limit per request  */
            pch->Err = MODBUS_ERR_FC04_03;
            MBS_ErrRespSet(pch,
                           MODBUS_ERR_ILLEGAL_DATA_QTY);
            return (true);                                   /* Tell caller that we need to send a response              */
        }
        nbr_bytes = (uint8_t)(nbr_regs * sizeof(uint16_t)); /* Find #bytes needed for response.                         */
    } else {
        if (nbr_regs == 0 || nbr_regs > 62) {                    /* Make sure we don't exceed the allowed limit per request  */
            pch->Err = MODBUS_ERR_FC04_04;
            MBS_ErrRespSet(pch,
                           MODBUS_ERR_ILLEGAL_DATA_QTY);
            return (true);                                   /* Tell caller that we need to send a response              */
        }
        nbr_bytes = (uint8_t)(nbr_regs * sizeof(float));   /* Find #bytes needed for response.                         */
    }
#else
    if (nbr_regs == 0 || nbr_regs > 125) {                       /* Make sure we don't exceed the allowed limit per request  */
        pch->Err = MODBUS_ERR_FC04_03;
        MBS_ErrRespSet(pch,
                       MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (true);                                       /* Tell caller that we need to send a response              */
    }
    nbr_bytes = (uint8_t)(nbr_regs * sizeof(uint16_t));     /* Find #bytes needed for response.                         */
#endif
    pch->TxFrameNDataBytes = nbr_bytes + 1;                      /* Number of data bytes + byte count.                       */
    presp                 = &pch->TxFrameData[0];                /* Reset the pointer to the start of the response           */
    *presp++              =  MBS_RX_FRAME_ADDR;                  /* Prepare response packet                                  */
    *presp++              =  MBS_RX_FRAME_FC;
    *presp++              = (uint8_t)nbr_bytes;               /* Set number of data bytes in response message             */
    while (nbr_regs > 0) {                                       /* Loop through each register requested.                    */
        if (reg < MODBUS_CFG_FP_START_IX)                       /* See if we want an integer register                       */
        {
            elab_assert(pch->cb.in_reg_read != NULL);
            /* Yes, get its value. */
            reg_val_16 = pch->cb.in_reg_read(reg, &err);
            switch (err) {
                case MODBUS_ERR_NONE:
                     *presp++ = (uint8_t)((reg_val_16 >> 8) & 0x00FF); /*      Get MSB first.                             */
                     *presp++ = (uint8_t)(reg_val_16 & 0x00FF);        /*      Get LSB next.                              */
                     break;

                case MODBUS_ERR_RANGE:
                default:
                     pch->Err = MODBUS_ERR_FC04_01;
                     MBS_ErrRespSet(pch,
                                    MODBUS_ERR_ILLEGAL_DATA_ADDR);
                     return (true);
            }
        } else {
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
            reg_val_fp = MB_InRegRdFP(reg,                       /* No,  get the value of the FP register                    */
                                      &err);
            switch (err) {
                case MODBUS_ERR_NONE:
                     pfp = (uint8_t *)&reg_val_fp;            /* Point to the FP register                                 */
#if CPU_CFG_ENDIAN_TYPE == CPU_ENDIAN_TYPE_BIG
                     for (ix = 0; ix < sizeof(float); ix++) { /* Copy value to response buffer                            */
                         *presp++ = *pfp++;
                     }
#else
                     pfp += sizeof(float) - 1;
                     for (ix = 0; ix < sizeof(float); ix++) {
                         *presp++ = *pfp--;
                     }
#endif
                     break;

                case MODBUS_ERR_RANGE:
                default:
                     pch->Err = MODBUS_ERR_FC04_02;
                     MBS_ErrRespSet(pch,
                                    MODBUS_ERR_ILLEGAL_DATA_ADDR);
                     return (true);
            }
#endif
        }
        reg++;                                                   /* Increment current register number                        */
        nbr_regs--;
    }
    pch->Err = MODBUS_ERR_NONE;
    return (true);                                           /* Tell caller that we need to send a response              */
}
#endif
#endif

/*
*********************************************************************************************************
*                                           MBS_FC05_CoilWr()
*
* Description : Responds to a request to force a coil to a specified state.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : MBS_FCxx_Handler().
*
* Note(s)     : 1) A value of 0xFF00 forces a coil ON and 0x0000 to OFF
*
*               2) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x05
*                  <Coil address HI>              0x00
*                  <Coil address LO>              0xAC
*                  <Force coil value HI>          0xFF
*                  <Force coil value LO>          0x00
*                  <Error Check (LRC or CRC)>     0x??
*
*               3) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x05
*                  <Coil address HI>              0x00
*                  <Coil address LO>              0xAC
*                  <Force coil value HI>          0xFF
*                  <Force coil value LO>          0x00
*                  <Error Check (LRC or CRC)>     0x??
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC05_EN  == DEF_ENABLED)
static  bool  MBS_FC05_CoilWr (elab_mb_channel_t  *pch)
{
    uint8_t   *prx_data;
    uint8_t   *ptx_data;
    uint8_t    i;
    uint16_t    coil;
    bool   coil_val;
    uint16_t    temp;
    uint16_t    err;


    if (pch->RxFrameNDataBytes != 4) {                           /* Nbr of data bytes must be 4.                             */
        return (false);                                      /* Tell caller that we DON'T need to send a response        */
    }
    coil = MBS_RX_DATA_START;                                    /* Get the desired coil number                              */
    temp = MBS_RX_DATA_COIL;
    if (pch->write_en == true) {
        if (temp == MODBUS_COIL_OFF_CODE) {                      /* See if coil needs to be OFF?                             */
            coil_val = 0;                                        /* Yes, Turn coil OFF                                       */
        } else {
            coil_val = 1;                                        /* No,  Turn coil ON                                        */
        }
        /* Force coil. */
        elab_assert(pch->cb.coil_write != NULL);
        pch->cb.coil_write(coil, coil_val, &err);
    } else {
        pch->Err = MODBUS_ERR_FC05_02;
        MBS_ErrRespSet(pch,                                      /* Writes are not enabled                                   */
                       MODBUS_ERR_ILLEGAL_DATA_VAL);
        return (true);                                       /* Tell caller that we need to send a response              */
    }
    pch->TxFrameNDataBytes = 4;
    MBS_TX_FRAME_ADDR      = MBS_RX_FRAME_ADDR;                  /* Prepare response packet                                  */
    MBS_TX_FRAME_FC        = MBS_RX_FRAME_FC;
    prx_data               = &pch->RxFrameData[2];               /* Copy four data bytes from the receive packet             */
    ptx_data               = &pch->TxFrameData[2];
    for (i = 0; i < 4; i++) {
        *ptx_data++ = *prx_data++;
    }
    switch (err) {
        case MODBUS_ERR_NONE:                                    /* We simply echoe back with the command received           */
             pch->Err = MODBUS_ERR_NONE;
             pch->WrCtr++;
             break;

        case MODBUS_ERR_RANGE:
        default:
             pch->Err = MODBUS_ERR_FC05_01;
             MBS_ErrRespSet(pch,
                            MODBUS_ERR_ILLEGAL_DATA_ADDR);
             break;
    }
    return (true);                                           /* Tell caller that we need to send a response              */
}
#endif
#endif

/*
*********************************************************************************************************
*                                        MBS_FC06_HoldingRegWr()
*
* Description : Change the value of a single register.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : MBS_FCxx_Handler().
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x06
*                  <start address HI>             0x00
*                  <start address LO>             0x01
*                  <Register value HI>            0x00
*                  <Register value LO>            0x03
*                  <Error Check (LRC or CRC)>     0x??
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x06
*                  <start address HI>             0x00
*                  <start address LO>             0x01
*                  <Register value HI>            0x00
*                  <Register value LO>            0x03
*                  <Error Check (LRC or CRC)>     0x??
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC06_EN  == DEF_ENABLED)
static  bool  MBS_FC06_HoldingRegWr (elab_mb_channel_t *pch)
{
    uint8_t  *prx_data;
    uint8_t  *ptx_data;
    uint8_t   i;
    uint8_t   max;
    uint16_t err;
    uint16_t reg = MBS_RX_DATA_START;
    uint16_t   reg_val_16;
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
    float     reg_val_fp = 1.0;
    uint8_t  *pfp;
#endif

    /* Nbr of data bytes must be 4. */
    if (pch->RxFrameNDataBytes != 4 && reg < MODBUS_CFG_FP_START_IX)
    {
        return (false);
    }

    /* Nbr of data bytes must be 6. */
    if (pch->RxFrameNDataBytes != 6 && reg >= MODBUS_CFG_FP_START_IX)
    {
        return (false);
    }
    
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
    if (reg < MODBUS_CFG_FP_START_IX)
    {
        reg_val_16 = MBS_RX_DATA_REG;
        /* Write to integer register. */
        elab_assert(pch->cb.holding_reg_write != NULL);
        pch->cb.holding_reg_write(reg, reg_val_16, &err);
    }
    else
    {
        prx_data = &pch->RxFrameData[4];                         /* Point to data in the received frame.                     */
        pfp      = (uint8_t *)&reg_val_fp;
#if CPU_CFG_ENDIAN_TYPE == CPU_ENDIAN_TYPE_BIG
        for (i = 0; i < sizeof(float); i++) {
            *pfp++ = *prx_data++;
        }
#else
        prx_data += sizeof(float) - 1;
        for (i = 0; i < sizeof(float); i++) {
            *pfp++ = *prx_data--;
        }
#endif
        /* Write to floating point register. */
        elab_assert(pch->cb.holding_reg_write_fp != NULL);
        pch->cb.holding_reg_write_fp(reg, reg_val_fp, &err);
    }
#else
    reg_val_16 = MBS_RX_DATA_REG;
    /* Write to integer register. */
    elab_assert(pch->cb.holding_reg_write != NULL);
    pch->cb.holding_reg_write(reg, reg_val_16, &err);
#endif
    pch->TxFrameNDataBytes = 4;
    MBS_TX_FRAME_ADDR      = MBS_RX_FRAME_ADDR;                  /* Prepare response packet (duplicate Rx frame)             */
    MBS_TX_FRAME_FC        = MBS_RX_FRAME_FC;
    prx_data               = &pch->RxFrameData[2];               /* Copy received register address and data to response      */
    ptx_data               = &pch->TxFrameData[2];
    if (reg < MODBUS_CFG_FP_START_IX) {
        max = sizeof(uint16_t) + 2;
    } else {
        max = sizeof(float) + 2;
    }
    for (i = 0; i < max; i++) {
        *ptx_data++ = *prx_data++;
    }
    switch (err) {
        case MODBUS_ERR_NONE:                                    /* Reply with echoe of command received                     */
             pch->Err = MODBUS_ERR_NONE;
             pch->WrCtr++;
             break;

        case MODBUS_ERR_RANGE:
        default:
             pch->Err = MODBUS_ERR_FC06_01;
             MBS_ErrRespSet(pch,
                            MODBUS_ERR_ILLEGAL_DATA_ADDR);
             break;
    }
    return (true);
}
#endif
#endif

/*
*********************************************************************************************************
*                                           MBS_FC08_Loopback()
*
* Description : The LOOPBACK function contains various diagnostic codes that perform specific actions.
*               This function processes individual diagnostic requests and formats the response message
*               frame accordingly.  Unimplemented diagnostic codes will return an Illegal Data Value
*               Exception Response Code (03).
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : MBS_FCxx_Handler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC08_EN  == DEF_ENABLED)
static  bool  MBS_FC08_Loopback (elab_mb_channel_t  *pch)
{
    uint16_t  diagcode;


    if (pch->RxFrameNDataBytes != 4) {                           /* Nbr of data bytes must be 4.                             */
        return (false);
    }
    diagcode           = MBS_RX_DIAG_CODE;
    MBS_TX_FRAME_ADDR  = MBS_RX_FRAME_ADDR;                      /* Prepare response packet                                  */
    MBS_TX_FRAME_FC    = MBS_RX_FRAME_FC;
    MBS_TX_DIAG_CODE_H = MBS_RX_DIAG_CODE_H;
    MBS_TX_DIAG_CODE_L = MBS_RX_DIAG_CODE_L;
    switch (diagcode) {
        case MODBUS_FC08_LOOPBACK_QUERY:                         /* Return Query function code - no need to do anything.     */
             MBS_TX_DIAG_DATA_H = MBS_RX_DIAG_DATA_H;
             MBS_TX_DIAG_DATA_L = MBS_RX_DIAG_DATA_L;
             pch->Err           = MODBUS_ERR_NONE;
             break;

        case MODBUS_FC08_LOOPBACK_CLR_CTR:
#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED) && \
    (MODBUS_CFG_FC08_EN  == DEF_ENABLED)
             MBS_StatInit(pch);                                  /* Initialize the system counters, echo response back.      */
#endif
             pch->Err = MODBUS_ERR_NONE;
             break;

        case MODBUS_FC08_LOOPBACK_BUS_MSG_CTR:                   /* Return the message count in diag information field.      */
             MBS_TX_DIAG_DATA_H = (uint8_t)((pch->StatMsgCtr & 0xFF00) >> 8);
             MBS_TX_DIAG_DATA_L = (uint8_t) (pch->StatMsgCtr & 0x00FF);
             pch->Err           = MODBUS_ERR_NONE;
             break;

        case MODBUS_FC08_LOOPBACK_BUS_CRC_CTR:                   /* Return the CRC error count in diag information field.    */
             MBS_TX_DIAG_DATA_H = (uint8_t)((pch->StatCRCErrCtr & 0xFF00) >> 8);
             MBS_TX_DIAG_DATA_L = (uint8_t) (pch->StatCRCErrCtr & 0x00FF);
             pch->Err           = MODBUS_ERR_NONE;
             break;

        case MODBUS_FC08_LOOPBACK_BUS_EXCEPT_CTR:                /* Return exception count in diag information field.        */
             MBS_TX_DIAG_DATA_H = (uint8_t)((pch->StatExceptCtr & 0xFF00) >> 8);
             MBS_TX_DIAG_DATA_L = (uint8_t) (pch->StatExceptCtr & 0x00FF);
             pch->Err           = MODBUS_ERR_NONE;
             break;

        case MODBUS_FC08_LOOPBACK_SLAVE_MSG_CTR:                 /* Return slave message count in diag information field.    */
             MBS_TX_DIAG_DATA_H = (uint8_t)((pch->StatSlaveMsgCtr & 0xFF00) >> 8);
             MBS_TX_DIAG_DATA_L = (uint8_t) (pch->StatSlaveMsgCtr & 0x00FF);
             pch->Err           = MODBUS_ERR_NONE;
             break;

        case MODBUS_FC08_LOOPBACK_SLAVE_NO_RESP_CTR:             /* Return no response count in diag information field.      */
             MBS_TX_DIAG_DATA_H = (uint8_t)((pch->StatNoRespCtr & 0xFF00) >> 8);
             MBS_TX_DIAG_DATA_L = (uint8_t) (pch->StatNoRespCtr & 0x00FF);
             pch->Err           = MODBUS_ERR_NONE;
             break;

        default:
             pch->Err = MODBUS_ERR_FC08_01;
             MBS_ErrRespSet(pch,
                            MODBUS_ERR_ILLEGAL_DATA_VAL);
             break;
    }
    return (true);                                           /* Tell caller that we need to send a response              */
}
#endif
#endif

/*
*********************************************************************************************************
*                                       MBS_FC15_CoilWrMultiple()
*
* Description : Processes the MODBUS "Force Multiple COILS" command and writes the COIL states.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : MBS_FCxx_Handler().
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x0F
*                  <Coil address HI>              0x00
*                  <Coil address LO>              0x13
*                  <# coils HI>                   0x00
*                  <# coils LO>                   0x0A
*                  <byte count>                   0x02
*                  <Force Data HI>                0xCD
*                  <Force Data LO>                0x01
*                  <Error Check (LRC or CRC)>     0x??
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x0F
*                  <Coil address HI>              0x00
*                  <Coil address LO>              0x13
*                  <# coils HI>                   0x00
*                  <# coils LO>                   0x0A
*                  <Error Check (LRC or CRC)>     0x??
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC15_EN  == DEF_ENABLED)
static  bool  MBS_FC15_CoilWrMultiple (elab_mb_channel_t  *pch)
{
    uint16_t   ix;
    uint16_t   coil;
    uint16_t   nbr_coils;
    uint16_t   nbr_bytes;
    uint8_t   data_ix;
    bool  coil_val;
    uint8_t   temp;
    uint16_t err;

    if (pch->write_en == true) {
        if (pch->RxFrameNDataBytes < 6) {                        /* Minimum Nbr of data bytes must be 6.                     */
            return (false);                                  /* Tell caller that we DON'T need to send a response        */
        }
        coil      = MBS_RX_DATA_START;
        nbr_coils = MBS_RX_DATA_POINTS;
        nbr_bytes = MBS_RX_DATA_BYTES;                           /* Get the byte count for the data.                         */
        if (((((nbr_coils - 1) / 8) + 1) ==  nbr_bytes) &&       /* Be sure #bytes valid for number COILS.                   */
            (pch->RxFrameNDataBytes  == (nbr_bytes + 5))) {
            ix      = 0;                                         /* Initialize COIL/loop counter variable.                   */
            data_ix = 7;                                         /* The 1st COIL data byte is 5th element in data frame.     */
            while (ix < nbr_coils) {                             /* Loop through each COIL to be forced.                     */
                if ((ix % 8) == 0) {                             /* Move to the next data byte after every eight bits.       */
                    temp = pch->RxFrameData[data_ix++];
                }
                if (temp & 0x01) {                               /* Get LSBit                                                */
                    coil_val = MODBUS_COIL_ON;
                } else {
                    coil_val = MODBUS_COIL_OFF;
                }
                elab_assert(pch->cb.coil_write != NULL);
                pch->cb.coil_write(coil + ix, coil_val, &err);
                switch (err) {
                    case MODBUS_ERR_NONE:
                         break;                                  /* Continue with the next coil if no error                  */

                    case MODBUS_ERR_RANGE:
                    default:
                         pch->Err = MODBUS_ERR_FC15_01;
                         MBS_ErrRespSet(pch,
                                        MODBUS_ERR_ILLEGAL_DATA_ADDR);
                         return (true);                      /* Tell caller that we need to send a response              */
                }
                temp >>= 1;                                      /* Shift the data one bit position to the right.            */
                ix++;                                            /* Increment the COIL counter.                              */
            }
        } else {
            pch->Err = MODBUS_ERR_FC15_02;
            MBS_ErrRespSet(pch,
                           MODBUS_ERR_ILLEGAL_DATA_VAL);
            return (true);                                   /* Tell caller that we need to send a response              */
        }
        pch->TxFrameNDataBytes = 4;                              /* Don't echo the whole message back!                       */
        MBS_TX_FRAME_ADDR      = MBS_RX_FRAME_ADDR;              /* Prepare response packet                                  */
        MBS_TX_FRAME_FC        = MBS_RX_FRAME_FC;
        MBS_TX_DATA_START_H    = MBS_RX_DATA_START_H;
        MBS_TX_DATA_START_L    = MBS_RX_DATA_START_L;
        MBS_TX_DATA_POINTS_H   = MBS_RX_DATA_POINTS_H;
        MBS_TX_DATA_POINTS_L   = MBS_RX_DATA_POINTS_L;
        pch->Err               = MODBUS_ERR_NONE;
    } else {
        pch->Err               = MODBUS_ERR_FC15_03;              /* Number of bytes incorrect for number of COILS.           */
        MBS_ErrRespSet(pch,
                       MODBUS_ERR_ILLEGAL_DATA_VAL);
    }
    return (true);                                            /* Tell caller that we need to send a response              */
}
#endif
#endif

/*
*********************************************************************************************************
*                                    MBS_FC16_HoldingRegWrMultiple()
*
* Description : This function is called to write to multiple holding registers.  If the address of the
*               rquest exceeds or is equal to MODBUS_CFG_FP_START_IX, then the command would write to
*               multiple 'floating-point' according to the 'Daniels Flow Meter' extensions.  This means
*               that each register requested is considered as a 32-bit IEEE-754 floating-point format.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : MBS_FCxx_Handler().
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x10
*                  <start address HI>             0x00
*                  <start address LO>             0x01
*                  <# registers HI>               0x00
*                  <# registers LO>               0x02
*                  <byte count>                   0x04
*                  <Register value HI>            0x00
*                  <Register value LO>            0x0A
*                  <Register value HI>            0x01
*                  <Register value LO>            0x02
*                  <Error Check (LRC or CRC)>     0x??
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x10
*                  <start address HI>             0x00
*                  <start address LO>             0x01
*                  <# registers HI>               0x00
*                  <# registers LO>               0x02
*                  <Error Check (LRC or CRC)>     0x??
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC16_EN  == DEF_ENABLED)
static  bool  MBS_FC16_HoldingRegWrMultiple (elab_mb_channel_t *pch)
{
    uint8_t  *prx_data;
    uint16_t err;
    uint16_t   reg;
    uint16_t   reg_val_16;
    uint16_t   nbr_regs;
    uint16_t   nbr_bytes;
    uint8_t   data_size;
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
    uint8_t   i;
    float     reg_val_fp;
    uint8_t  *pfp;
#endif


    reg       = MBS_RX_DATA_START;
    nbr_regs  = MBS_RX_DATA_POINTS;
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
    if (reg < MODBUS_CFG_FP_START_IX) {
        if (nbr_regs == 0 || nbr_regs > 125) {                   /* Make sure we don't exceed the allowed limit per request  */
            pch->Err = MODBUS_ERR_FC16_04;
            MBS_ErrRespSet(pch,
                           MODBUS_ERR_ILLEGAL_DATA_QTY);
            return (true);                                   /* Tell caller that we need to send a response              */
        }
        data_size  = sizeof(uint16_t);
    } else {
        if (nbr_regs == 0 || nbr_regs > 62) {                    /* Make sure we don't exceed the allowed limit per request  */
            pch->Err = MODBUS_ERR_FC16_05;
            MBS_ErrRespSet(pch,
                           MODBUS_ERR_ILLEGAL_DATA_QTY);
            return (true);                                   /* Tell caller that we need to send a response              */
        }
        data_size  = sizeof(float);
    }
#else
    if (nbr_regs == 0 || nbr_regs > 125) {                       /* Make sure we don't exceed the allowed limit per request  */
        pch->Err = MODBUS_ERR_FC16_04;
        MBS_ErrRespSet(pch,
                       MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (true);                                       /* Tell caller that we need to send a response              */
    }
    data_size  = sizeof(uint16_t);
#endif

    prx_data  = &pch->RxFrameData[6];                            /* Point to number of bytes in request frame                */
    nbr_bytes = (uint16_t)*prx_data++;
    if ((pch->RxFrameNDataBytes - 5) != nbr_bytes) {             /* Compare actual number of bytes to what they say.         */
        pch->Err = MODBUS_ERR_FC16_01;
        MBS_ErrRespSet(pch,
                       MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (true);
    }
    if ((nbr_bytes / nbr_regs) != (uint16_t)data_size) {
        pch->Err = MODBUS_ERR_FC16_02;
        MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_VAL);
        return (true);                                       /* Tell caller that we need to send a response              */
    }
    while (nbr_regs > 0) {
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
        if (reg < MODBUS_CFG_FP_START_IX) {
            reg_val_16  = ((uint16_t)*prx_data++) << 8;        /* Get MSB first.                                           */
            reg_val_16 +=  (uint16_t)*prx_data++;              /* Add in the LSB.                                          */
            elab_assert(pch->cb.holding_reg_write != NULL);
            pch->cb.holding_reg_write(reg, reg_val_16, &err);
        }
        else
        {
            pfp = (uint8_t *)&reg_val_fp;
  #if CPU_CFG_ENDIAN_TYPE == CPU_ENDIAN_TYPE_BIG
            for (i = 0; i < sizeof(float); i++) {
                *pfp++ = *prx_data++;
            }
  #else
            pfp += sizeof(float) - 1;
            for (i = 0; i < sizeof(float); i++) {
                *pfp-- = *prx_data++;
            }
  #endif
            elab_assert(pch->cb.holding_reg_write_fp != NULL);
            pch->cb.holding_reg_write_fp(reg, reg_val_fp, &err);
        }
#else
        reg_val_16  = ((uint16_t)*prx_data++) << 8;            /* Get MSB first.                                           */
        reg_val_16 +=  (uint16_t)*prx_data++;                  /* Add in the LSB.                                          */
        elab_assert(pch->cb.holding_reg_write != NULL);
        pch->cb.holding_reg_write(reg, reg_val_16, &err);
#endif

        switch (err) {                                           /* See if any errors in writing the data                    */
            case MODBUS_ERR_NONE:                                /* Reply with echoe of command received                     */
                 pch->WrCtr++;
                 reg++;
                 nbr_regs--;
                 break;

            case MODBUS_ERR_RANGE:
            default:
                 pch->Err = MODBUS_ERR_FC16_03;
                 MBS_ErrRespSet(pch,
                                MODBUS_ERR_ILLEGAL_DATA_ADDR);
                 return (true);                              /* Tell caller that we need to send a response              */
        }
    }
    pch->TxFrameNDataBytes = 4;                                  /* Don't echo the whole message back!                       */
    MBS_TX_FRAME_ADDR      = MBS_RX_FRAME_ADDR;                  /* Prepare response packet                                  */
    MBS_TX_FRAME_FC        = MBS_RX_FRAME_FC;
    MBS_TX_DATA_START_H    = MBS_RX_DATA_START_H;
    MBS_TX_DATA_START_L    = MBS_RX_DATA_START_L;
    MBS_TX_DATA_POINTS_H   = MBS_RX_DATA_POINTS_H;
    MBS_TX_DATA_POINTS_L   = MBS_RX_DATA_POINTS_L;
    return (true);                                           /* Tell caller that we need to send a response              */
}
#endif
#endif

/*
*********************************************************************************************************
*                                          MBS_FC20_FileRd()
*
* Description : Read a record from a file.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : MBS_FCxx_Handler().
*
* Note(s)     : 1) The current version of this software only supports ONE Sub-request at a time.
*
*               2) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x14
*                  <byte count>                   0x0E
*                  <Reference Type>               0x06
*                  <File number HI>               0x00
*                  <File number LO>               0x04
*                  <Starting address HI>          0x00
*                  <Starting address LO>          0x01
*                  <Register count HI>            0x00
*                  <Register count LO>            0x02
*                  <Error Check (LRC or CRC)>     0x??
*
*               3) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x14
*                  <byte count>                   0x0C
*                  <start address HI>             0x00
*                  <Reference Type>               0x06
*                  <Register data HI>             0x0D
*                  <Register data LO>             0xFE
*                  <Register data HI>             0x00
*                  <Register data LO>             0x20
*                  <Error Check (LRC or CRC)>     0x??
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC20_EN  == DEF_ENABLED)
static  bool  MBS_FC20_FileRd (elab_mb_channel_t  *pch)
{
    uint8_t  *presp;
    uint16_t   file_nbr;
    uint16_t   record_nbr;
    uint16_t   record_len;
    uint16_t   cmd_len;
    uint8_t   cmd_type;
    uint16_t err;
    uint16_t   reg_val;
    uint16_t   ix;


    cmd_len = pch->RxFrameData[2];                                           /* Get the number of bytes in the command received          */
    if (cmd_len < 7 || cmd_len > 245) {                                      /* Make sure the byte count Rx'd is within expected range   */
        pch->Err = MODBUS_ERR_FC20_01;
        MBS_ErrRespSet(pch,
                       MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (true);                                                   /* Tell caller that we need to send a response              */
    }
    cmd_type    =  pch->RxFrameData[3];                                      /* Get the reference type                                   */
    file_nbr    = ((uint16_t)pch->RxFrameData[4] << 8)                     /* Get the file number                                      */
                +  (uint16_t)pch->RxFrameData[5];
    record_nbr  = ((uint16_t)pch->RxFrameData[6] << 8)                     /* Get the record number                                    */
                +  (uint16_t)pch->RxFrameData[7];
    record_len  = ((uint16_t)pch->RxFrameData[8] << 8)                     /* Get the record length                                    */
                +  (uint16_t)pch->RxFrameData[9];
    presp       = &pch->TxFrameData[0];                                      /* Point to first location in response buffer               */
    *presp++    = MBS_RX_FRAME_ADDR;                                         /* Reply back with the node address                         */
    *presp++    = MBS_RX_FRAME_FC;                                           /* Include the function code                                */
    if (cmd_type == 6) {                                                     /* File type should ALWAYS be 6.                            */
        pch->TxFrameNDataBytes = record_len * sizeof(uint16_t) + 3;        /* Determine the total number of data bytes in the Tx frame */
        *presp++               = (uint8_t)(pch->TxFrameNDataBytes - 1);   /* Total byte count (excluding byte count)                  */
        *presp++               = (uint8_t)(pch->TxFrameNDataBytes - 2);   /* Sub request byte count (excluding sub-request byte cnt)  */
        *presp++               = 6;                                          /* Reference type is ALWAYS 6.                              */
        ix                     = 0;                                          /* Initialize the index into the record                     */
        while (record_len > 0) {
            reg_val = elab_mb_file_read(file_nbr,                                    /* Get one value from the file                              */
                                record_nbr,
                                ix,
                                record_len,
                                &err);
            switch (err) {
                case MODBUS_ERR_NONE:
                     *presp++ = (uint8_t)(reg_val >> 8);                  /* Store high byte of record data                           */
                     *presp++ = (uint8_t)(reg_val & 0x00FF);              /* Store low  byte of record data                           */
                     break;

                case MODBUS_ERR_FILE:
                     pch->Err = MODBUS_ERR_FC20_02;
                     MBS_ErrRespSet(pch,
                                    MODBUS_ERR_ILLEGAL_DATA_ADDR);
                     return (true);                                      /* Tell caller that we need to send a response              */

                case MODBUS_ERR_RECORD:
                     pch->Err = MODBUS_ERR_FC20_03;
                     MBS_ErrRespSet(pch,
                                    MODBUS_ERR_ILLEGAL_DATA_ADDR);
                     return (true);                                      /* Tell caller that we need to send a response              */

                case MODBUS_ERR_IX:
                default:
                     pch->Err = MODBUS_ERR_FC20_04;
                     MBS_ErrRespSet(pch,
                                    MODBUS_ERR_ILLEGAL_DATA_ADDR);
                     return (true);                                      /* Tell caller that we need to send a response              */
            }
            ix++;
            record_len--;
        }
    } else {
        pch->Err = MODBUS_ERR_FC20_05;
        MBS_ErrRespSet(pch,
                       MODBUS_ERR_ILLEGAL_DATA_ADDR);
        return (true);                                                   /* Tell caller that we need to send a response              */
    }
    pch->Err = MODBUS_ERR_NONE;
    return (true);                                                       /* Tell caller that we need to send a response              */
}
#endif
#endif

/*
*********************************************************************************************************
*                                       MBS_FC21_FileWr()
*
* Description : Write a record to a file.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : true      If a response needs to be sent
*               false     If not
*
* Caller(s)   : MBS_FCxx_Handler().
*
* Note(s)     : (1) The current version of this software only supports ONE Sub-request at a time.
*
*               2) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x15
*                  <byte count>                   0x0D
*                  <Reference Type>               0x06
*                  <File number HI>               0x00
*                  <File number LO>               0x04
*                  <Starting address HI>          0x00
*                  <Starting address LO>          0x07
*                  <Register count HI>            0x00
*                  <Register count LO>            0x03
*                  <Register data HI>             0x06
*                  <Register data LO>             0xAF
*                  <Register data HI>             0x04
*                  <Register data LO>             0xBE
*                  <Register data HI>             0x10
*                  <Register data LO>             0x0D
*                  <Error Check (LRC or CRC)>     0x??
*
*               3) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x15
*                  <byte count>                   0x0D
*                  <Reference Type>               0x06
*                  <File number HI>               0x00
*                  <File number LO>               0x04
*                  <Starting address HI>          0x00
*                  <Starting address LO>          0x07
*                  <Register count HI>            0x00
*                  <Register count LO>            0x03
*                  <Register data HI>             0x06
*                  <Register data LO>             0xAF
*                  <Register data HI>             0x04
*                  <Register data LO>             0xBE
*                  <Register data HI>             0x10
*                  <Register data LO>             0x0D
*                  <Error Check (LRC or CRC)>     0x??
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC21_EN  == DEF_ENABLED)
static  bool  MBS_FC21_FileWr (elab_mb_channel_t  *pch)
{
    uint8_t  *prx_data;
    uint8_t  *pcmd;
    uint8_t  *presp;
    uint16_t   file_nbr;
    uint16_t   record_nbr;
    uint16_t   record_len;
    uint16_t   cmd_len;
    uint8_t   cmd_type;
    uint16_t err;
    uint8_t   max;
    uint16_t   reg_val;
    uint16_t   ix;


    cmd_len = pch->RxFrameData[2];
    if (cmd_len < 7 || cmd_len > 245) {                                    /* Make sure the byte count Rx'd is within expected range   */
        pch->Err = MODBUS_ERR_FC21_01;
        MBS_ErrRespSet(pch,
                       MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (true);                                                 /* Tell caller that we need to send a response              */
    }
    cmd_type    =  pch->RxFrameData[3];                                    /* Get the reference type                                   */
    file_nbr    = ((uint16_t)pch->RxFrameData[4] << 8)                   /* Get the file number                                      */
                +  (uint16_t)pch->RxFrameData[5];
    record_nbr  = ((uint16_t)pch->RxFrameData[6] << 8)                   /* Get the record number                                    */
                + (uint16_t)pch->RxFrameData[7];
    record_len  = ((uint16_t)pch->RxFrameData[8] << 8)                   /* Get the record length                                    */
                + (uint16_t)pch->RxFrameData[9];
    prx_data    = &pch->RxFrameData[10];                                   /* Point to first data byte                                 */

    if (cmd_type == 6) {                                                   /* File type should ALWAYS be 6.                            */
        ix = 0;                                                            /* Initialize the index into the record                     */
        while (record_len > 0) {
            reg_val  = ((uint16_t)*prx_data++ << 8) & 0xFF00;            /* Get data to write to file                                */
            reg_val |=  (uint16_t)*prx_data++ & 0x00FF;
            elab_mb_file_write(file_nbr,                                            /* Write one value to the file                              */
                      record_nbr,
                      ix,
                      record_len,
                      reg_val,
                      &err);
            switch (err) {
                case MODBUS_ERR_NONE:
                     pch->WrCtr++;
                     break;

                case MODBUS_ERR_FILE:
                     pch->Err = MODBUS_ERR_FC21_02;
                     MBS_ErrRespSet(pch,
                                    MODBUS_ERR_ILLEGAL_DATA_ADDR);
                     return (true);                                    /* Tell caller that we need to send a response              */

                case MODBUS_ERR_RECORD:
                     pch->Err = MODBUS_ERR_FC21_03;
                     MBS_ErrRespSet(pch,
                                    MODBUS_ERR_ILLEGAL_DATA_ADDR);
                     return (true);                                    /* Tell caller that we need to send a response              */

                case MODBUS_ERR_IX:
                default:
                     pch->Err = MODBUS_ERR_FC21_04;
                     MBS_ErrRespSet(pch,
                                    MODBUS_ERR_ILLEGAL_DATA_ADDR);
                     return (true);                                    /* Tell caller that we need to send a response              */
            }
            ix++;
            record_len--;
        }
    } else {
        pch->Err = MODBUS_ERR_FC21_05;
        MBS_ErrRespSet(pch,
                       MODBUS_ERR_ILLEGAL_DATA_ADDR);
        return (true);
    }
    record_len = ((uint16_t)pch->RxFrameData[8] << 8) + (uint16_t)pch->RxFrameData[9];    /* Get the record length                 */
    pcmd       = &pch->RxFrameData[0];
    presp      = &pch->TxFrameData[0];                                     /* Point to first location in response buffer               */
    max        = (record_len * 2) + 9;
    for (ix = 0; ix < max; ix++) {                                         /* Copy the request into the transmit packet                */
        *presp++ = *pcmd++;
    }
    pch->Err = MODBUS_ERR_NONE;
    return (true);                                                     /* Tell caller that we need to send a response              */
}
#endif
#endif

/*
*********************************************************************************************************
*                                            MBS_StatInit()
*
* Description : This function is used to initialize/reset the MODBUS statistics/communications counters.
*
* Argument(s) : pch      Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : none.
*
* Caller(s)   : elab_mb_init(),
*               MBS_FC08_Loopback().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED) && \
    (MODBUS_CFG_FC08_EN  == DEF_ENABLED)
void  MBS_StatInit (elab_mb_channel_t  *pch)
{
    pch->StatMsgCtr      = 0;                       /* Initialize all MODBUS event counters.                     */
    pch->StatCRCErrCtr   = 0;
    pch->StatExceptCtr   = 0;
    pch->StatSlaveMsgCtr = 0;
    pch->StatNoRespCtr   = 0;
}
#endif

/*
*********************************************************************************************************
*                                           MBS_RxTask()
*
* Description : Handle either Modbus ASCII or Modbus RTU received packets.
*
* Argument(s) : ch       Specifies the Modbus channel that needs servicing.
*
* Return(s)   : none.
*
* Caller(s)   : MB_RxTask().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
void  MBS_RxTask (elab_mb_channel_t *pch)
{
    if (pch != (elab_mb_channel_t *)0) {
#if (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
        if (pch->Mode == MODBUS_MODE_ASCII) {
            MBS_ASCII_Task(pch);
        }
#endif

#if (MODBUS_CFG_RTU_EN != 0)
        if (pch->Mode == MODBUS_MODE_RTU) {
            MBS_RTU_Task(pch);
        }
#endif
    }
}
#endif

/*
*********************************************************************************************************
*                                           MBS_ASCII_Task()
*
* Description : Received a packet that should be encoded for Modbus ASCII mode.  Process request.
*
* Argument(s) : pch         Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : none.
*
* Caller(s)   : MBS_RxTask().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED) && \
    (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
static  void  MBS_ASCII_Task (elab_mb_channel_t  *pch)
{
    bool ok;
    uint16_t   calc_lrc;                                /* Used for LRC                                                    */
    bool  send_reply;


#if (MODBUS_CFG_FC08_EN == DEF_ENABLED)
    pch->StatMsgCtr++;
#endif
    if (pch->RxBufByteCtr >= MODBUS_ASCII_MIN_MSG_SIZE) {
        ok = MB_ASCII_Rx(pch);                            /* Extract received command from .RxBuf[] & move to .RxFrameData[] */
        if (ok == true) {
            calc_lrc = MB_ASCII_RxCalcLRC(pch);           /* Calculate LRC on received ASCII packet                          */
            if (calc_lrc != pch->RxFrameCRC) {            /* If sum of all data plus received LRC is not the same            */
#if (MODBUS_CFG_FC08_EN == DEF_ENABLED)
                pch->StatCRCErrCtr++;                     /* then the frame was not received properly.                       */
                pch->StatNoRespCtr++;
#endif
            } else {
                send_reply = MBS_FCxx_Handler(pch);       /* Execute received command and formulate a response               */
                if (send_reply == true) {
                    MB_ASCII_Tx(pch);                     /* Send back reply.                                                */
                } else {
#if (MODBUS_CFG_FC08_EN == DEF_ENABLED)
                    pch->StatNoRespCtr++;
#endif
                }
            }
        } else {
#if (MODBUS_CFG_FC08_EN == DEF_ENABLED)
            pch->StatNoRespCtr++;
#endif
        }
    }
    pch->RxBufByteCtr = 0;
    pch->RxBufPtr     = &pch->RxBuf[0];
}
#endif

/*
*********************************************************************************************************
*                                            MBS_RTU_Task()
*
* Description : This function processes a packet received on the Modbus channel assuming that it's an RTU
*               packet.
*
* Argument(s) : pch      Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : none.
*
* Caller(s)   : MBS_RTU_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED) && \
    (MODBUS_CFG_RTU_EN   == DEF_ENABLED)
static  void  MBS_RTU_Task (elab_mb_channel_t  *pch)
{
    bool ok;
    uint16_t   calc_crc;                             /* Used for CRC                                                    */
    bool  send_reply;



#if (MODBUS_CFG_FC08_EN == DEF_ENABLED)
    pch->StatMsgCtr++;
#endif
    if (pch->RxBufByteCtr >= MODBUS_RTU_MIN_MSG_SIZE) {
        ok = MB_RTU_Rx(pch);                           /* Extract received command from .RxBuf[] & move to .RxFrameData[] */
        if (ok == true) {
            calc_crc = MB_RTU_RxCalcCRC(pch);          /* Do our own calculation of the CRC.                              */
            if (calc_crc != pch->RxFrameCRC) {         /* If the calculated CRC does not match the CRC received,          */
#if (MODBUS_CFG_FC08_EN == DEF_ENABLED)
                pch->StatCRCErrCtr++;                  /* then the frame is bad.                                          */
                pch->StatNoRespCtr++;
#endif
            } else {
                send_reply = MBS_FCxx_Handler(pch);    /* Execute received command and formulate a response               */
                if (send_reply == true) {
                    MB_RTU_Tx(pch);                    /* Send back reply.                                                */
                } else {
#if (MODBUS_CFG_FC08_EN == DEF_ENABLED)
                    pch->StatNoRespCtr++;
#endif
                }
            }
        }
    }
    pch->RxBufByteCtr = 0;
    pch->RxBufPtr     = &pch->RxBuf[0];
}
#endif
#endif