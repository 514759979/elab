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
*                                     uC/MODBUS MASTER COMMAND PROCESSOR
*
* Filename : mbm_core.c
* Version  : V2.14.00
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define    MBM_MODULE
#include  "mb.h"
#include "../../common/elab_assert.h"

ELAB_TAG("MbmCore");


#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)

#define  MBM_TX_FRAME                         (&pch->TxFrame)
#define  MBM_TX_FRAME_NBYTES                  (pch->TxFrameNDataBytes)

#define  MBM_TX_FRAME_SLAVE_ADDR              (pch->TxFrameData[0])
#define  MBM_TX_FRAME_FC                      (pch->TxFrameData[1])

#define  MBM_TX_FRAME_FC01_ADDR_HI            (pch->TxFrameData[2])
#define  MBM_TX_FRAME_FC01_ADDR_LO            (pch->TxFrameData[3])
#define  MBM_TX_FRAME_FC01_NBR_POINTS_HI      (pch->TxFrameData[4])
#define  MBM_TX_FRAME_FC01_NBR_POINTS_LO      (pch->TxFrameData[5])

#define  MBM_TX_FRAME_FC02_ADDR_HI            (pch->TxFrameData[2])
#define  MBM_TX_FRAME_FC02_ADDR_LO            (pch->TxFrameData[3])
#define  MBM_TX_FRAME_FC02_NBR_POINTS_HI      (pch->TxFrameData[4])
#define  MBM_TX_FRAME_FC02_NBR_POINTS_LO      (pch->TxFrameData[5])

#define  MBM_TX_FRAME_FC03_ADDR_HI            (pch->TxFrameData[2])
#define  MBM_TX_FRAME_FC03_ADDR_LO            (pch->TxFrameData[3])
#define  MBM_TX_FRAME_FC03_NBR_POINTS_HI      (pch->TxFrameData[4])
#define  MBM_TX_FRAME_FC03_NBR_POINTS_LO      (pch->TxFrameData[5])

#define  MBM_TX_FRAME_FC04_ADDR_HI            (pch->TxFrameData[2])
#define  MBM_TX_FRAME_FC04_ADDR_LO            (pch->TxFrameData[3])
#define  MBM_TX_FRAME_FC04_NBR_POINTS_HI      (pch->TxFrameData[4])
#define  MBM_TX_FRAME_FC04_NBR_POINTS_LO      (pch->TxFrameData[5])

#define  MBM_TX_FRAME_FC05_ADDR_HI            (pch->TxFrameData[2])
#define  MBM_TX_FRAME_FC05_ADDR_LO            (pch->TxFrameData[3])
#define  MBM_TX_FRAME_FC05_FORCE_DATA_HI      (pch->TxFrameData[4])
#define  MBM_TX_FRAME_FC05_FORCE_DATA_LO      (pch->TxFrameData[5])

#define  MBM_TX_FRAME_FC06_ADDR_HI            (pch->TxFrameData[2])
#define  MBM_TX_FRAME_FC06_ADDR_LO            (pch->TxFrameData[3])
#define  MBM_TX_FRAME_FC06_DATA_ADDR         (&pch->TxFrameData[4])
#define  MBM_TX_FRAME_FC06_DATA_HI            (pch->TxFrameData[4])
#define  MBM_TX_FRAME_FC06_DATA_LO            (pch->TxFrameData[5])

#define  MBM_TX_FRAME_FC08_FNCT_HI            (pch->TxFrameData[2])
#define  MBM_TX_FRAME_FC08_FNCT_LO            (pch->TxFrameData[3])
#define  MBM_TX_FRAME_FC08_FNCT_DATA_HI       (pch->TxFrameData[4])
#define  MBM_TX_FRAME_FC08_FNCT_DATA_LO       (pch->TxFrameData[5])

#define  MBM_TX_FRAME_FC15_ADDR_HI            (pch->TxFrameData[2])
#define  MBM_TX_FRAME_FC15_ADDR_LO            (pch->TxFrameData[3])
#define  MBM_TX_FRAME_FC15_NBR_POINTS_HI      (pch->TxFrameData[4])
#define  MBM_TX_FRAME_FC15_NBR_POINTS_LO      (pch->TxFrameData[5])
#define  MBM_TX_FRAME_FC15_BYTE_CNT           (pch->TxFrameData[6])
#define  MBM_TX_FRAME_FC15_DATA              (&pch->TxFrameData[7])

#define  MBM_TX_FRAME_FC16_ADDR_HI            (pch->TxFrameData[2])
#define  MBM_TX_FRAME_FC16_ADDR_LO            (pch->TxFrameData[3])
#define  MBM_TX_FRAME_FC16_NBR_REGS_HI        (pch->TxFrameData[4])
#define  MBM_TX_FRAME_FC16_NBR_REGS_LO        (pch->TxFrameData[5])
#define  MBM_TX_FRAME_FC16_BYTE_CNT           (pch->TxFrameData[6])
#define  MBM_TX_FRAME_FC16_DATA              (&pch->TxFrameData[7])


#define  MBM_TX_FRAME_DIAG_FNCT_HI            (pch->TxFrameData[2])
#define  MBM_TX_FRAME_DIAG_FNCT_LO            (pch->TxFrameData[3])
#define  MBM_TX_FRAME_DIAG_FNCT_DATA_HI       (pch->TxFrameData[4])
#define  MBM_TX_FRAME_DIAG_FNCT_DATA_LO       (pch->TxFrameData[5])



/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if  (MODBUS_CFG_FC01_EN == DEF_ENABLED) || \
     (MODBUS_CFG_FC02_EN == DEF_ENABLED)
static  uint16_t   MBM_Coil_DI_Rd_Resp(elab_mb_channel_t   *pch,
                                         uint8_t  *ptbl);
#endif

#if  (MODBUS_CFG_FC03_EN == DEF_ENABLED) || \
     (MODBUS_CFG_FC04_EN == DEF_ENABLED)
static  uint16_t   MBM_RegRd_Resp     (elab_mb_channel_t   *pch,
                                         uint16_t  *ptbl);
#endif

#if  (MODBUS_CFG_FC03_EN == DEF_ENABLED) && \
     (MODBUS_CFG_FP_EN   == DEF_ENABLED)
static  uint16_t   MBM_RegRdFP_Resp   (elab_mb_channel_t   *pch,
                                         float    *ptbl);
#endif

#if   (MODBUS_CFG_FC05_EN == DEF_ENABLED)
static  uint16_t   MBM_CoilWr_Resp    (elab_mb_channel_t   *pch);
#endif

#if   (MODBUS_CFG_FC15_EN == DEF_ENABLED)
static  uint16_t   MBM_CoilWrN_Resp   (elab_mb_channel_t   *pch);
#endif

#if   (MODBUS_CFG_FC06_EN == DEF_ENABLED)
static  uint16_t   MBM_RegWr_Resp     (elab_mb_channel_t   *pch);
#endif

#if   (MODBUS_CFG_FC16_EN == DEF_ENABLED)
static  uint16_t   MBM_RegWrN_Resp    (elab_mb_channel_t   *pch);
#endif

#if   (MODBUS_CFG_FC08_EN == DEF_ENABLED)
static  uint16_t   MBM_Diag_Resp      (elab_mb_channel_t   *pch,
                                         uint16_t  *pval);
#endif

static  bool  MBM_RxReply        (elab_mb_channel_t   *pch);

static  void         MBM_TxCmd          (elab_mb_channel_t   *pch);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                            MBM_FC01_CoilRd()
*
* Description : Sends a MODBUS message to read the status of coils from a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to obtain the information from.
*
*               slave_addr       Is the Modbus coil start address
*
*               p_coil_tbl       Is a pointer to an array of bytes containing the value of the coils read.  The
*                                format is:
*                                                MSB                               LSB
*                                                B7   B6   B5   B4   B3   B2   B1   B0
*                                                -------------------------------------
*                                p_coil_tbl[0]   #8   #7                            #1
*                                p_coil_tbl[1]   #16  #15                           #9
*                                     :
*                                     :
*
*                                Note that the array that will be receiving the coil values must be greater
*                                than or equal to:   (nbr_coils - 1) / 8 + 1
*
*               nbr_coils        Is the desired number of coils to read
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC01_EN == DEF_ENABLED)
uint16_t  MBM_FC01_CoilRd (elab_mb_channel_t   *pch,
                             uint8_t   slave_addr,
                             uint16_t   start_addr,
                             uint8_t  *p_coil_tbl,
                             uint16_t   nbr_coils)
{
    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    uint16_t err;
    bool ok;

    MBM_TX_FRAME_NBYTES             = 4;
    MBM_TX_FRAME_SLAVE_ADDR         = slave_addr;                               /* Slave Address                     */
    MBM_TX_FRAME_FC                 = 1;                                        /* Function Code                     */
    MBM_TX_FRAME_FC01_ADDR_HI       = (uint8_t)((start_addr >> 8) & 0x00FF); /* Staring Address                   */
    MBM_TX_FRAME_FC01_ADDR_LO       = (uint8_t) (start_addr       & 0x00FF);
    MBM_TX_FRAME_FC01_NBR_POINTS_HI = (uint8_t)((nbr_coils  >> 8) & 0x00FF); /* Number of points                  */
    MBM_TX_FRAME_FC01_NBR_POINTS_LO = (uint8_t) (nbr_coils        & 0x00FF);

    MBM_TxCmd(pch);                                                             /* Send command to the slave         */

    elab_mb_os_rx_wait(pch,                                                           /* Wait for response from slave      */
                 &err);

    if (err == MODBUS_ERR_NONE) {
        ok = MBM_RxReply(pch);
        if (ok == MODBUS_TRUE) {
            err = MBM_Coil_DI_Rd_Resp(pch,                                      /* Parse the response from the slave */
                                      p_coil_tbl);
        } else {
            err = MODBUS_ERR_RX;
        }
    }

    pch->RxBufByteCtr = 0;
    pch->RxBufPtr     = &pch->RxBuf[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif


/*
*********************************************************************************************************
*                                             MBM_FC02_DIRd()
*
* Description : Sends a MODBUS message to read the status of discrete inputs from a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to obtain the information from.
*
*               slave_addr       Is the Modbus discrete input start address
*
*               p_di_tbl         Is a pointer to an array that will receive the state of the desired discrete inputs.
*                                The format of the array is as follows:
*
*                                              MSB                               LSB
*                                              B7   B6   B5   B4   B3   B2   B1   B0
*                                              -------------------------------------
*                                p_di_tbl[0]   #8   #7                            #1
*                                p_di_tbl[1]   #16  #15                           #9
*                                     :
*                                     :
*
*                                Note that the array that will be receiving the discrete input values must be greater
*                                than or equal to:   (nbr_di - 1) / 8 + 1
*
*               nbr_di           Is the desired number of discrete inputs to read
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC02_EN == DEF_ENABLED)
uint16_t  MBM_FC02_DIRd (elab_mb_channel_t   *pch,
                           uint8_t   slave_node,
                           uint16_t   slave_addr,
                           uint8_t  *p_di_tbl,
                           uint16_t   nbr_di)
{
    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    uint16_t err;
    bool ok;

    MBM_TX_FRAME_NBYTES             = 4;
    MBM_TX_FRAME_SLAVE_ADDR         = slave_node;                               /* Setup command                     */
    MBM_TX_FRAME_FC                 = 2;
    MBM_TX_FRAME_FC02_ADDR_HI       = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC02_ADDR_LO       = (uint8_t) (slave_addr       & 0x00FF);
    MBM_TX_FRAME_FC02_NBR_POINTS_HI = (uint8_t)((nbr_di     >> 8) & 0x00FF);
    MBM_TX_FRAME_FC02_NBR_POINTS_LO = (uint8_t) (nbr_di           & 0x00FF);

    MBM_TxCmd(pch);                                                             /* Send command to the slave         */

    elab_mb_os_rx_wait(pch,                                                           /* Wait for response from slave      */
                 &err);

    if (err == MODBUS_ERR_NONE) {
        ok = MBM_RxReply(pch);
        if (ok == MODBUS_TRUE) {
            err = MBM_Coil_DI_Rd_Resp(pch,                                      /* Parse the response from the slave */
                                      p_di_tbl);
        } else {
            err = MODBUS_ERR_RX;
        }
    }

    pch->RxBufByteCtr = 0;
    pch->RxBufPtr     = &pch->RxBuf[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif


/*
*********************************************************************************************************
*                                       MBM_FC03_HoldingRegRd()
*
* Description : Sends a MODBUS message to read the value of holding registers from a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to obtain the information from.
*
*               slave_addr       Is the Modbus holding register start address
*
*               p_reg_tbl        Is a pointer to an array of integers that will receive the current value of
*                                the desired holding registers from the slave.  The array pointed to by
*                                'p_reg_tbl' needs to be able to hold at least 'nbr_regs' entries.
*
*               nbr_regs         Is the desired number of holding registers to read
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC03_EN == DEF_ENABLED)
uint16_t  MBM_FC03_HoldingRegRd (elab_mb_channel_t   *pch,
                                   uint8_t   slave_node,
                                   uint16_t   slave_addr,
                                   uint16_t  *p_reg_tbl,
                                   uint16_t   nbr_regs)
{
    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    uint16_t err;
    bool ok;

    MBM_TX_FRAME_NBYTES             = 4;
    MBM_TX_FRAME_SLAVE_ADDR         = slave_node;                               /* Setup command                     */
    MBM_TX_FRAME_FC                 = 3;
    MBM_TX_FRAME_FC03_ADDR_HI       = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC03_ADDR_LO       = (uint8_t) (slave_addr       & 0x00FF);
    MBM_TX_FRAME_FC03_NBR_POINTS_HI = (uint8_t)((nbr_regs   >> 8) & 0x00FF);
    MBM_TX_FRAME_FC03_NBR_POINTS_LO = (uint8_t) (nbr_regs         & 0x00FF);

    MBM_TxCmd(pch);                                                             /* Send command to the slave         */

    elab_mb_os_rx_wait(pch,                                                           /* Wait for response from slave      */
                 &err);

    if (err == MODBUS_ERR_NONE) {
        ok = MBM_RxReply(pch);
        if (ok == MODBUS_TRUE) {
            err = MBM_RegRd_Resp(pch,                                           /* Parse the response from the slave */
                                 p_reg_tbl);
        } else {
            err = MODBUS_ERR_RX;
        }
    }

    pch->RxBufByteCtr = 0;
    pch->RxBufPtr     = &pch->RxBuf[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif


/*
*********************************************************************************************************
*                                       MBM_FC03_HoldingRegRdFP()
*
* Description : Sends a MODBUS message to read the value of floating-point holding registers from a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to obtain the information from.
*
*               slave_addr       Is the Modbus floating-point holding register start address
*
*               p_reg_tbl        Is a pointer to an array of floating-points that will receive the current
*                                value of the desired holding registers from the slave.  The array pointed to
*                                by 'p_reg_tbl' needs to be able to hold at least 'nbr_regs' entries.
*
*               nbr_regs         Is the desired number of holding registers to read
*
* Return(s)   : MODBUS_ERR_NONE  If the function was sucessful.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC03_EN == DEF_ENABLED) && \
    (MODBUS_CFG_FP_EN   == DEF_ENABLED)
uint16_t  MBM_FC03_HoldingRegRdFP (elab_mb_channel_t   *pch,
                                     uint8_t   slave_node,
                                     uint16_t   slave_addr,
                                     float    *p_reg_tbl,
                                     uint16_t   nbr_regs)
{
    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    uint16_t err;
    bool ok;

    MBM_TX_FRAME_NBYTES             = 4;
    MBM_TX_FRAME_SLAVE_ADDR         = slave_node;                               /* Setup command                     */
    MBM_TX_FRAME_FC                 = 3;
    MBM_TX_FRAME_FC03_ADDR_HI       = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC03_ADDR_LO       = (uint8_t) (slave_addr       & 0x00FF);
    MBM_TX_FRAME_FC03_NBR_POINTS_HI = (uint8_t)((nbr_regs   >> 8) & 0x00FF);
    MBM_TX_FRAME_FC03_NBR_POINTS_LO = (uint8_t) (nbr_regs         & 0x00FF);

    MBM_TxCmd(pch);                                                             /* Send command to the slave         */

    elab_mb_os_rx_wait(pch,                                                           /* Wait for response from slave      */
                 &err);

    if (err == MODBUS_ERR_NONE) {
        ok = MBM_RxReply(pch);
        if (ok == MODBUS_TRUE) {
            err = MBM_RegRdFP_Resp(pch,                                         /* Parse the response from the slave */
                                   p_reg_tbl);
        } else {
            err = MODBUS_ERR_RX;
        }
    }

    pch->RxBufByteCtr = 0;
    pch->RxBufPtr     = &pch->RxBuf[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif


/*
*********************************************************************************************************
*                                          MBM_FC04_InRegRd()
*
* Description : Sends a MODBUS message to read the value of input registers from a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to obtain the information from.
*
*               slave_addr       Is the Modbus input register start address
*
*               p_reg_tbl        Is a pointer to an array of integers that will receive the current value of
*                                the desired holding registers from the slave.  The array pointed to by
*                                'p_reg_tbl' needs to be able to hold at least 'nbr_regs' entries.
*
*               nbr_regs         Is the desired number of input registers to read
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC04_EN == DEF_ENABLED)
uint16_t  MBM_FC04_InRegRd (elab_mb_channel_t   *pch,
                              uint8_t   slave_node,
                              uint16_t   slave_addr,
                              uint16_t  *p_reg_tbl,
                              uint16_t   nbr_regs)
{
    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    uint16_t err;
    bool ok;

    MBM_TX_FRAME_NBYTES             = 4;
    MBM_TX_FRAME_SLAVE_ADDR         = slave_node;                               /* Setup command                     */
    MBM_TX_FRAME_FC                 = 4;
    MBM_TX_FRAME_FC04_ADDR_HI       = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC04_ADDR_LO       = (uint8_t) (slave_addr       & 0x00FF);
    MBM_TX_FRAME_FC04_NBR_POINTS_HI = (uint8_t)((nbr_regs   >> 8) & 0x00FF);
    MBM_TX_FRAME_FC04_NBR_POINTS_LO = (uint8_t) (nbr_regs         & 0x00FF);

    MBM_TxCmd(pch);                                                             /* Send command to the slave         */

    elab_mb_os_rx_wait(pch,                                                           /* Wait for response from slave      */
                 &err);

    if (err == MODBUS_ERR_NONE) {
        ok = MBM_RxReply(pch);
        if (ok == MODBUS_TRUE) {
            err = MBM_RegRd_Resp(pch,                                           /* Parse the response from the slave */
                                 p_reg_tbl);
        } else {
            err = MODBUS_ERR_RX;
        }
    }

    pch->RxBufByteCtr = 0;
    pch->RxBufPtr     = &pch->RxBuf[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif

/*
*********************************************************************************************************
*                                          MBM_FC44_InRegRd()
*
*                                      Just for Inventus Battery
*
* Description : Sends a MODBUS message to read the value of input registers from a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to obtain the information from.
*
*               slave_addr       Is the Modbus input register start address
*
*               p_reg_tbl        Is a pointer to an array of integers that will receive the current value of
*                                the desired holding registers from the slave.  The array pointed to by
*                                'p_reg_tbl' needs to be able to hold at least 'nbr_regs' entries.
*
*               nbr_regs         Is the desired number of input registers to read
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              elab_mbm_fc05_wirte_coil()
*
* Description : Sends a MODBUS message to write the value of single coil to a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to obtain the information from.
*
*               slave_addr       Is the Modbus coil to change
*
*               coil_val         Is the desired value of the coil:
*                                   MODBUS_COIL_ON   to turn the coil on.
*                                   MODBUS_COIL_OFF  to turn the coil off.
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*               MODBUS_ERR_COIL_ADDR     If you specified an invalid coil address
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC05_EN == DEF_ENABLED)
uint16_t elab_mbm_fc05_wirte_coil(elab_mb_channel_t *pch,
                                    uint8_t slave_node,
                                    uint16_t slave_addr,
                                    bool coil_val)
{
    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    uint16_t err;
    bool ok;

    MBM_TX_FRAME_NBYTES        = 4;
    MBM_TX_FRAME_SLAVE_ADDR    = slave_node;                                    /* Setup command                     */
    MBM_TX_FRAME_FC            = 5;
    MBM_TX_FRAME_FC05_ADDR_HI  = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC05_ADDR_LO  = (uint8_t) (slave_addr       & 0x00FF);
    if (coil_val == MODBUS_COIL_OFF)
    {
        MBM_TX_FRAME_FC05_FORCE_DATA_HI = (uint8_t)0x00;
        MBM_TX_FRAME_FC05_FORCE_DATA_LO = (uint8_t)0x00;
    }
    else
    {
        MBM_TX_FRAME_FC05_FORCE_DATA_HI = (uint8_t)0xFF;
        MBM_TX_FRAME_FC05_FORCE_DATA_LO = (uint8_t)0x00;
    }

    /* Send command to the slave. */
    MBM_TxCmd(pch);
    /* Wait for response from slave */
    elab_mb_os_rx_wait(pch, &err);

    if (err == MODBUS_ERR_NONE) {
        ok = MBM_RxReply(pch);
        if (ok == MODBUS_TRUE) {
            /* Parse the response from the slave */
            err = MBM_CoilWr_Resp(pch);
        } else {
            err = MODBUS_ERR_RX;
        }
    }

    pch->RxBufByteCtr = 0;
    pch->RxBufPtr     = &pch->RxBuf[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif


/*
*********************************************************************************************************
*                                          MBM_FC06_HoldingRegWr()
*
* Description : Sends a MODBUS message to write the value of single holding register to a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to obtain the information from.
*
*               slave_addr       Is the Modbus holding register address
*
*               reg_val          Is the desired 'integer' value of the holding register:
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_REG_ADDR      If you specified an invalid register address
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC06_EN == DEF_ENABLED)
uint16_t MBM_FC06_HoldingRegWr(elab_mb_channel_t   *pch,
                                uint8_t   slave_node,
                                uint16_t   slave_addr,
                                uint16_t   reg_val)
{
    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    uint16_t err;
    bool ok;

    MBM_TX_FRAME_NBYTES       = 4;
    MBM_TX_FRAME_SLAVE_ADDR   = slave_node;                                     /* Setup command                     */
    MBM_TX_FRAME_FC           = 6;
    MBM_TX_FRAME_FC06_ADDR_HI = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC06_ADDR_LO = (uint8_t) (slave_addr       & 0x00FF);
    MBM_TX_FRAME_FC06_DATA_HI = (uint8_t)((reg_val >> 8)    & 0x00FF);
    MBM_TX_FRAME_FC06_DATA_LO = (uint8_t) (reg_val          & 0x00FF);

    MBM_TxCmd(pch);                                                             /* Send command to the slave         */

    elab_mb_os_rx_wait(pch,                                                           /* Wait for response from slave      */
                 &err);

    if (err == MODBUS_ERR_NONE) {
        ok = MBM_RxReply(pch);
        if (ok == MODBUS_TRUE) {
            err = MBM_RegWr_Resp(pch);                                          /* Parse the response from the slave */
        } else {
            err = MODBUS_ERR_RX;
        }
    }

    pch->RxBufByteCtr = 0;
    pch->RxBufPtr     = &pch->RxBuf[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}

uint16_t  MBM_FC06_HoldingRegWr_NoAck(elab_mb_channel_t   *pch,
                                      uint8_t   slave_node,
                                      uint16_t   slave_addr,
                                      uint16_t   reg_val,
                                      uint32_t timeout_ms)
{
    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    uint16_t err;
    bool ok;

    MBM_TX_FRAME_NBYTES       = 4;
    MBM_TX_FRAME_SLAVE_ADDR   = slave_node;                                     /* Setup command                     */
    MBM_TX_FRAME_FC           = 6;
    MBM_TX_FRAME_FC06_ADDR_HI = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC06_ADDR_LO = (uint8_t) (slave_addr       & 0x00FF);
    MBM_TX_FRAME_FC06_DATA_HI = (uint8_t)((reg_val >> 8)    & 0x00FF);
    MBM_TX_FRAME_FC06_DATA_LO = (uint8_t) (reg_val          & 0x00FF);

    pch->no_ack = true;
    pch->no_ack_timeout_ms = timeout_ms;

    MBM_TxCmd(pch);                                                             /* Send command to the slave         */

    pch->no_ack = 0;
    pch->no_ack_timeout_ms = 0;

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif


/*
*********************************************************************************************************
*                                         MBM_FC06_HoldingRegWrFP()
*
* Description : Sends a MODBUS message to write the value of single holding register to a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to obtain the information from.
*
*               slave_addr       Is the Modbus holding register address
*
*               reg_val_fp       Is the desired value of the floating-point holding register:
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_REG_ADDR      If you specified an invalid register address
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC06_EN == DEF_ENABLED) && \
    (MODBUS_CFG_FP_EN   == DEF_ENABLED)
uint16_t MBM_FC06_HoldingRegWrFP(elab_mb_channel_t *pch,
                                     uint8_t   slave_node,
                                     uint16_t   slave_addr,
                                     float     reg_val_fp)
{
    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    uint16_t err;
    bool ok;
    uint8_t   i;
    uint8_t  *p_fp;
    uint8_t  *p_data;

    MBM_TX_FRAME_NBYTES       = 6;
    MBM_TX_FRAME_SLAVE_ADDR   = slave_node;             /* Setup command. */
    MBM_TX_FRAME_FC           = 6;
    MBM_TX_FRAME_FC06_ADDR_HI = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC06_ADDR_LO = (uint8_t) (slave_addr       & 0x00FF);

    /* Point to the FP value. */
    p_fp   = (uint8_t *)&reg_val_fp;
    p_data = MBM_TX_FRAME_FC06_DATA_ADDR;

#if CPU_CFG_ENDIAN_TYPE == CPU_ENDIAN_TYPE_BIG
    /* Copy value to transmit buffer. */
    for (i = 0; i < sizeof(float); i++)
    {
        *p_data++ = *p_fp++;
        
    }
#else
    p_fp += sizeof(float) - 1;
    for (i = 0; i < sizeof(float); i++) {
        *p_data++ = *p_fp--;
    }
#endif

    /* Send command. */
    MBM_TxCmd(pch);

    /* Wait for response from slave. */
    elab_mb_os_rx_wait(pch, &err);

    if (err == MODBUS_ERR_NONE)
    {
        ok = MBM_RxReply(pch);
        if (ok == true)
        {
            /* Parse the response from the slave */
            err = MBM_RegWr_Resp(pch);
        }
        else {
            err = MODBUS_ERR_RX;
        }
    }

    pch->RxBufByteCtr = 0;
    pch->RxBufPtr     = &pch->RxBuf[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif


/*
*********************************************************************************************************
*                                            MBM_FC08_Diag()
*
* Description : Sends a MODBUS message to perform a diagnostic function of a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to obtain the information from.
*
*               fnct             Is a sub-function code
*
*               fnct_data        Is the data for a sub-function (if needed)
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SUB_FNCT      If you specified an invalid sub-function code
*               MODBUS_ERR_DIAG          If there was an error in the command
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC08_EN == DEF_ENABLED)
uint16_t  MBM_FC08_Diag (elab_mb_channel_t   *pch,
                           uint8_t   slave_node,
                           uint16_t   fnct,
                           uint16_t   fnct_data,
                           uint16_t  *pval)
{
    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    uint16_t err;
    bool ok;

    MBM_TX_FRAME_NBYTES            = 4;
    MBM_TX_FRAME_SLAVE_ADDR        = slave_node;                                /* Setup command                     */
    MBM_TX_FRAME_FC                = 8;
    MBM_TX_FRAME_FC08_FNCT_HI      = (uint8_t)((fnct       >> 8) & 0x00FF);
    MBM_TX_FRAME_FC08_FNCT_LO      = (uint8_t) (fnct             & 0x00FF);
    MBM_TX_FRAME_FC08_FNCT_DATA_HI = (uint8_t)((fnct_data  >> 8) & 0x00FF);
    MBM_TX_FRAME_FC08_FNCT_DATA_LO = (uint8_t) (fnct_data        & 0x00FF);

    MBM_TxCmd(pch);                                                             /* Send command                      */

    elab_mb_os_rx_wait(pch,                                                           /* Wait for response from slave      */
                 &err);

    if (err == MODBUS_ERR_NONE) {
        ok = MBM_RxReply(pch);
        if (ok == MODBUS_TRUE) {
            err = MBM_Diag_Resp(pch,                                            /* Parse the response from the slave */
                                pval);
        } else {
            err = MODBUS_ERR_RX;
        }
    } else {
        *pval = 0;
    }

    pch->RxBufByteCtr = 0;
    pch->RxBufPtr     = &pch->RxBuf[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif


/*
*********************************************************************************************************
*                                            MBM_FC15_CoilWr()
*
* Description : Sends a MODBUS message to write to coils on a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to write to.
*
*               slave_addr       Is the Modbus coil start address
*
*               p_coil_tbl       Is a pointer to an array of bytes containing the value of the coils to write.
*                                The format is:
*
*                                                MSB                               LSB
*                                                B7   B6   B5   B4   B3   B2   B1   B0
*                                                -------------------------------------
*                                p_coil_tbl[0]   #8   #7                            #1
*                                p_coil_tbl[1]   #16  #15                           #9
*                                     :
*                                     :
*
*                                Note that the array containing the coil values must be greater than or equal
*                                to:   nbr_coils / 8 + 1
*
*               nbr_coils        Is the desired number of coils to write
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC15_EN == DEF_ENABLED)
uint16_t  MBM_FC15_CoilWr (elab_mb_channel_t   *pch,
                             uint8_t   slave_node,
                             uint16_t   slave_addr,
                             uint8_t  *p_coil_tbl,
                             uint16_t   nbr_coils)
{
    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    uint16_t err;
    uint8_t   nbr_bytes;
    uint8_t   i;
    uint8_t  *p_data;

    MBM_TX_FRAME_NBYTES             =  6;
    MBM_TX_FRAME_SLAVE_ADDR         = slave_node;       /* Setup command. */
    MBM_TX_FRAME_FC                 = 15;
    MBM_TX_FRAME_FC15_ADDR_HI       = (uint8_t) ((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC15_ADDR_LO       = (uint8_t)  (slave_addr       & 0x00FF);
    MBM_TX_FRAME_FC15_NBR_POINTS_HI = (uint8_t) ((nbr_coils  >> 8) & 0x00FF);
    MBM_TX_FRAME_FC15_NBR_POINTS_LO = (uint8_t)  (nbr_coils        & 0x00FF);
    nbr_bytes                       = (uint8_t)(((nbr_coils - 1) / 8) + 1);
    MBM_TX_FRAME_FC15_BYTE_CNT      = nbr_bytes;
    p_data                          = MBM_TX_FRAME_FC15_DATA;

    for (i = 0; i < nbr_bytes; i++)
    {
        *p_data++ = *p_coil_tbl++;
    }

    MBM_TxCmd(pch);                                     /* Send command. */

    /* Wait for response from slave. */
    elab_mb_os_rx_wait(pch, &err);

    if (err == MODBUS_ERR_NONE)
    {
        bool ok = MBM_RxReply(pch);
        if (ok == MODBUS_TRUE)
        {
            /* Parse the response from the slave */
            err = MBM_CoilWrN_Resp(pch);
        }
        else
        {
            err = MODBUS_ERR_RX;
        }
    }

    pch->RxBufByteCtr = 0;
    pch->RxBufPtr = &pch->RxBuf[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif


/*
*********************************************************************************************************
*                                       MBM_FC16_HoldingRegWrN()
*
* Description : Sends a MODBUS message to write to integer holding registers to a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to write to.
*
*               slave_addr       Is the Modbus holding register start address
*
*               p_reg_tbl        Is a pointer to an array of integers containing the value of the holding
*                                registers to write.
*
*                                Note that the array containing the register values must be greater than or equal
*                                to 'nbr_regs'
*
*               nbr_regs         Is the desired number of registers to write
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_REG_ADDR      If you specified an invalid register address
*               MODBUS_ERR_NBR_REG       If you specified an invalid number of registers
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC16_EN == DEF_ENABLED)
uint16_t  MBM_FC16_HoldingRegWrN (elab_mb_channel_t   *pch,
                                    uint8_t   slave_node,
                                    uint16_t   slave_addr,
                                    uint16_t  *p_reg_tbl,
                                    uint16_t   nbr_regs)
{
    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    uint16_t err;
    bool ok;
    uint8_t   nbr_bytes;
    uint8_t   i;
    uint8_t  *p_data;

    MBM_TX_FRAME_NBYTES             =  nbr_regs * sizeof(uint16_t) + 5;
    MBM_TX_FRAME_SLAVE_ADDR         = slave_node;                               /* Setup command                     */
    MBM_TX_FRAME_FC                 = 16;
    MBM_TX_FRAME_FC16_ADDR_HI       = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC16_ADDR_LO       = (uint8_t) (slave_addr       & 0x00FF);
    MBM_TX_FRAME_FC16_NBR_REGS_HI   = (uint8_t)((nbr_regs   >> 8) & 0x00FF);
    MBM_TX_FRAME_FC16_NBR_REGS_LO   = (uint8_t) (nbr_regs         & 0x00FF);
    nbr_bytes                       = (uint8_t) (nbr_regs * sizeof(uint16_t));
    MBM_TX_FRAME_FC16_BYTE_CNT      = nbr_bytes;
    p_data                          = MBM_TX_FRAME_FC16_DATA;

    for (i = 0; i < nbr_bytes; i++) {
        *p_data++ = (uint8_t)((*p_reg_tbl >> 8) & 0x00FF);                   /* Write HIGH data byte              */
        *p_data++ = (uint8_t) (*p_reg_tbl       & 0x00FF);                   /* Write LOW  data byte              */
        p_reg_tbl++;
    }

    MBM_TxCmd(pch);                                                             /* Send command                      */

    elab_mb_os_rx_wait(pch,                                                           /* Wait for response from slave      */
                 &err);

    if (err == MODBUS_ERR_NONE) {
        ok = MBM_RxReply(pch);
        if (ok == MODBUS_TRUE) {
            err = MBM_RegWrN_Resp(pch);                                         /* Parse the response from the slave */
        } else {
            err = MODBUS_ERR_RX;
        }
    }

    pch->RxBufByteCtr = 0;
    pch->RxBufPtr     = &pch->RxBuf[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif


/*
*********************************************************************************************************
*                                       MBM_FC16_HoldingRegWrNFP()
*
* Description : Sends a MODBUS message to write to floating-point holding registers to a slave unit.
*
* Argument(s) : pch              Is a pointer to the Modbus channel to send the request to.
*
*               slave_node       Is the Modbus node number of the desired slave to write to.
*
*               slave_addr       Is the Modbus holding register start address
*
*               p_reg_tbl        Is a pointer to an array of floating-points containing the value of the holding
*                                registers to write.
*
*                                Note that the array containing the register values must be greater than or equal
*                                to 'nbr_regs'
*
*               nbr_regs         Is the desired number of registers to write
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_RX            If a timeout occurred before receiving a response from the slave.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_REG_ADDR      If you specified an invalid register address
*               MODBUS_ERR_NBR_REG       If you specified an invalid number of registers
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC16_EN == DEF_ENABLED) && \
    (MODBUS_CFG_FP_EN   == DEF_ENABLED)
uint16_t  MBM_FC16_HoldingRegWrNFP (elab_mb_channel_t   *pch,
                                      uint8_t   slave_node,
                                      uint16_t   slave_addr,
                                      float    *p_reg_tbl,
                                      uint16_t   nbr_regs)
{
    osStatus_t ret_os = osOK;
    ret_os = osMutexAcquire(pch->mutex, osWaitForever);
    elab_assert(ret_os == osOK);

    uint16_t err;
    bool ok;
    uint8_t   nbr_bytes;
    uint8_t   i;
    uint16_t   n;
    uint8_t  *p_data;
    float    *p_fp;

    MBM_TX_FRAME_NBYTES           =  4;
    MBM_TX_FRAME_SLAVE_ADDR       = slave_node;                                 /* Setup command                     */
    MBM_TX_FRAME_FC               = 16;
    MBM_TX_FRAME_FC16_ADDR_HI     = (uint8_t)((slave_addr >> 8) & 0x00FF);
    MBM_TX_FRAME_FC16_ADDR_LO     = (uint8_t) (slave_addr       & 0x00FF);
    MBM_TX_FRAME_FC16_NBR_REGS_HI = (uint8_t)((nbr_regs   >> 8) & 0x00FF);
    MBM_TX_FRAME_FC16_NBR_REGS_LO = (uint8_t) (nbr_regs         & 0x00FF);
    nbr_bytes                     = (uint8_t) (nbr_regs * sizeof(float));
    MBM_TX_FRAME_FC16_BYTE_CNT    = nbr_bytes;
    p_data                        = MBM_TX_FRAME_FC16_DATA;

    for (n = 0; n < nbr_regs; n++) {                                            /* Copy all floating point values    */
        p_fp = p_reg_tbl;
#if CPU_CFG_ENDIAN_TYPE == CPU_ENDIAN_TYPE_BIG
        for (i = 0; i < sizeof(float); i++) {
            *p_data++ = *p_fp++;
        }
#else
        p_fp += sizeof(float) - 1;
        for (i = 0; i < sizeof(float); i++) {
            *p_data++ = (uint8_t)*p_fp--;
        }
#endif
        p_reg_tbl++;
    }

    MBM_TxCmd(pch);                                                             /* Send command                      */

    elab_mb_os_rx_wait(pch,                                                           /* Wait for response from slave      */
                 &err);

    if (err == MODBUS_ERR_NONE) {
        ok = MBM_RxReply(pch);
        if (ok == true) {
            err = MBM_RegWrN_Resp(pch);                                         /* Parse the response from the slave */
        } else {
            err = MODBUS_ERR_RX;
        }
    }

    pch->RxBufByteCtr = 0;
    pch->RxBufPtr     = &pch->RxBuf[0];

    ret_os = osMutexRelease(pch->mutex);
    elab_assert(ret_os == osOK);

    return (err);
}
#endif


/*
*********************************************************************************************************
*                                       MBM_Coil_DI_Rd_Resp()
*
* Description : Checks the slave's response to a request to read the status of coils or discrete inputs.
*
* Argument(s) : pch       A pointer to the channel that the message was received on.
*
*               ptbl      Pointer to where data will be placed
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : MBM_FC01_CoilRd(),
*               MBM_FC02_DIRd().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if(MODBUS_CFG_FC01_EN == DEF_ENABLED) || \
   (MODBUS_CFG_FC02_EN == DEF_ENABLED)
static  uint16_t  MBM_Coil_DI_Rd_Resp (elab_mb_channel_t *pch, uint8_t *ptbl)
{
    uint8_t       slave_addr;
    uint8_t       fnct_code;
    uint8_t       byte_cnt;
    uint8_t       nbr_points;
    uint8_t       i;
    uint8_t      *psrc;

    slave_addr = MBM_TX_FRAME_SLAVE_ADDR;                     /* Validate slave address                      */
    if (slave_addr != pch->RxFrameData[0]) {
        return (MODBUS_ERR_SLAVE_ADDR);
    }

    fnct_code  = MBM_TX_FRAME_FC;                             /* Validate function code                      */
    if (fnct_code != pch->RxFrameData[1]) {
        return (MODBUS_ERR_FC);
    }
                                                              /* Validate number of bytes received           */
    nbr_points = (MBM_TX_FRAME_FC01_NBR_POINTS_HI << 8)
               +  MBM_TX_FRAME_FC01_NBR_POINTS_LO;
    byte_cnt   = (uint8_t)((nbr_points - 1) / 8) + 1;
    if (byte_cnt != pch->RxFrameData[2]) {
        return (MODBUS_ERR_BYTE_COUNT);
    }

    psrc = &pch->RxFrameData[3];                              /* Copy received data to destination array     */
    for (i = 0; i < byte_cnt; i++) {
        *ptbl++ = *psrc++;
    }

    return (MODBUS_ERR_NONE);
}
#endif

/*
*********************************************************************************************************
*                                          MBM_RegRd_Resp()
*
* Description : Checks the slave's response to a request to read the status of input or output registers.
*
* Argument(s) : pch             A pointer to the channel that the message was received on.
*
*               ptbl            A pointer to where data will be placed
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC03_EN == DEF_ENABLED) || \
    (MODBUS_CFG_FC04_EN == DEF_ENABLED)
static  uint16_t  MBM_RegRd_Resp (elab_mb_channel_t *pch, uint16_t *ptbl)
{
    uint8_t       slave_addr;
    uint8_t       fnct_code;
    uint8_t       byte_cnt;
    uint8_t       nbr_points;
    uint8_t       data_hi;
    uint8_t       data_lo;
    uint8_t       i;
    uint8_t      *psrc;



    slave_addr = MBM_TX_FRAME_SLAVE_ADDR;                     /* Validate slave address                      */
    if (slave_addr != pch->RxFrameData[0]) {
        return (MODBUS_ERR_SLAVE_ADDR);
    }

    fnct_code  = MBM_TX_FRAME_FC;                             /* Validate function code                      */
    if (fnct_code != pch->RxFrameData[1]) {
        return (MODBUS_ERR_FC);
    }
                                                              /* Validate number of bytes received           */
    nbr_points = (MBM_TX_FRAME_FC01_NBR_POINTS_HI << 8)
               +  MBM_TX_FRAME_FC01_NBR_POINTS_LO;
    byte_cnt   = nbr_points * sizeof(uint16_t);
    if (byte_cnt != pch->RxFrameData[2]) {
        return (MODBUS_ERR_BYTE_COUNT);
    }

    psrc = &pch->RxFrameData[3];                              /* Copy received data to destination array     */
    for (i = 0; i < byte_cnt; i += sizeof(uint16_t)) {
        data_hi = *psrc++;
        data_lo = *psrc++;
        *ptbl++ = ((uint16_t)data_hi << 8) + (uint16_t)data_lo;
    }

    return (MODBUS_ERR_NONE);
}
#endif

/*
*********************************************************************************************************
*                                          MBM_RegRdFP_Resp()
*
* Description : Checks the slave's response to a request to read the status of input or output registers.
*
* Argument(s) : pch             A pointer to the channel that the message was received on.
*
*               ptbl            A pointer to where data will be placed
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : MBM_FC03_HoldingRegRdFP().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC03_EN == DEF_ENABLED) && \
    (MODBUS_CFG_FP_EN   == DEF_ENABLED)
static  uint16_t  MBM_RegRdFP_Resp (elab_mb_channel_t *pch, float *ptbl)
{
    uint8_t       slave_addr;
    uint8_t       fnct_code;
    uint8_t       byte_cnt;
    uint8_t       nbr_points;
    uint8_t       i;
    uint8_t       j;
    uint8_t      *psrc;
    uint8_t      *pdest;



    slave_addr = MBM_TX_FRAME_SLAVE_ADDR;                     /* Validate slave address                      */
    if (slave_addr != pch->RxFrameData[0]) {
        return (MODBUS_ERR_SLAVE_ADDR);
    }

    fnct_code   = MBM_TX_FRAME_FC;                            /* Validate function code                      */
    if (fnct_code != pch->RxFrameData[1]) {
        return (MODBUS_ERR_FC);
    }
                                                              /* Validate number of bytes received           */
    nbr_points = (MBM_TX_FRAME_FC01_NBR_POINTS_HI << 8)
               +  MBM_TX_FRAME_FC01_NBR_POINTS_LO;
    byte_cnt   = nbr_points * sizeof(float);
    if (byte_cnt != pch->RxFrameData[2]) {
        return (MODBUS_ERR_BYTE_COUNT);
    }

    psrc  = &pch->RxFrameData[3];                             /* Copy received data to destination array     */
    pdest = (uint8_t *)ptbl;
    for (i = 0; i < byte_cnt; i += sizeof(float)) {
#if CPU_CFG_ENDIAN_TYPE == CPU_ENDIAN_TYPE_BIG
        for (j = 0; j < sizeof(float); j++) {
            *pdest++ = *psrc++;
        }
#else
        pdest += sizeof(float) - 1;
        for (j = 0; j < sizeof(float); j++) {
            *pdest-- = *psrc++;
        }
        pdest += sizeof(float) + 1;
#endif
    }

    return (MODBUS_ERR_NONE);
}
#endif

/*
*********************************************************************************************************
*                                          MBM_CoilWr_Resp()
*
* Description : Checks the slave's response to a request to write coils.
*
* Argument(s) : pch             A pointer to the channel that the message was received on.
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*               MODBUS_ERR_COIL_ADDR     If you specified an invalid coil address
*
* Caller(s)   : elab_mbm_fc05_wirte_coil().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC05_EN == DEF_ENABLED)
static  uint16_t  MBM_CoilWr_Resp (elab_mb_channel_t *pch)
{
    uint8_t   slave_addr;
    uint8_t   fnct_code;
    uint8_t   coil_addr_hi;
    uint8_t   coil_addr_lo;

    slave_addr = MBM_TX_FRAME_SLAVE_ADDR;                     /* Validate slave address                      */
    if (slave_addr != pch->RxFrameData[0]) {
        return (MODBUS_ERR_SLAVE_ADDR);
    }

    fnct_code  = MBM_TX_FRAME_FC;                             /* Validate function code                      */
    if (fnct_code != pch->RxFrameData[1]) {
        return (MODBUS_ERR_FC);
    }

    coil_addr_hi = MBM_TX_FRAME_FC05_ADDR_HI;                  /* Validate function code                      */
    coil_addr_lo = MBM_TX_FRAME_FC05_ADDR_LO;
    if ((coil_addr_hi != pch->RxFrameData[2]) || (coil_addr_lo != pch->RxFrameData[3])) {
        return (MODBUS_ERR_COIL_ADDR);
    }

    return (MODBUS_ERR_NONE);
}
#endif

/*
*********************************************************************************************************
*                                          MBM_CoilWrN_Resp()
*
* Description : Checks the slave's response to a request to write coils.
*
* Argument(s) : pch             A pointer to the channel that the message was received on.
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : MBM_FC15_CoilWr().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC15_EN == DEF_ENABLED)
static  uint16_t  MBM_CoilWrN_Resp (elab_mb_channel_t *pch)
{
    uint8_t   slave_addr;
    uint8_t   fnct_code;
    uint8_t   coil_addr_hi;
    uint8_t   coil_addr_lo;
    uint8_t   coil_qty_hi;
    uint8_t   coil_qty_lo;



    slave_addr = MBM_TX_FRAME_SLAVE_ADDR;                     /* Validate slave address                      */
    if (slave_addr != pch->RxFrameData[0]) {
        return (MODBUS_ERR_SLAVE_ADDR);
    }

    fnct_code  = MBM_TX_FRAME_FC;                             /* Validate function code                      */
    if (fnct_code != pch->RxFrameData[1]) {
        return (MODBUS_ERR_FC);
    }

    coil_addr_hi = MBM_TX_FRAME_FC05_ADDR_HI;                 /* Validate coil address                       */
    coil_addr_lo = MBM_TX_FRAME_FC05_ADDR_LO;
    if ((coil_addr_hi != pch->RxFrameData[2]) || (coil_addr_lo != pch->RxFrameData[3])) {
        return (MODBUS_ERR_COIL_ADDR);
    }

    coil_qty_hi  = MBM_TX_FRAME_FC15_NBR_POINTS_HI;           /* Validate number of coils                    */
    coil_qty_lo  = MBM_TX_FRAME_FC15_NBR_POINTS_LO;
    if ((coil_qty_hi != pch->RxFrameData[4]) || (coil_qty_lo != pch->RxFrameData[5])) {
        return (MODBUS_ERR_COIL_QTY);
    }

    return (MODBUS_ERR_NONE);
}
#endif

/*
*********************************************************************************************************
*                                            MBM_RegWr_Resp()
*
* Description : Checks the slave's response to a request to write a register.
*
* Argument(s) : pch             A pointer to the channel that the message was received on.
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : MBM_FC06_HoldingRegWr(),
*               MBM_FC06_HoldingRegWrFP().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC06_EN == DEF_ENABLED)
static  uint16_t  MBM_RegWr_Resp (elab_mb_channel_t *pch)
{
    uint8_t   slave_addr;
    uint8_t   fnct_code;
    uint8_t   reg_addr_hi;
    uint8_t   reg_addr_lo;



    slave_addr = MBM_TX_FRAME_SLAVE_ADDR;                     /* Validate slave address                      */
    if (slave_addr != pch->RxFrameData[0]) {
        return (MODBUS_ERR_SLAVE_ADDR);
    }

    fnct_code  = MBM_TX_FRAME_FC;                             /* Validate function code                      */
    if (fnct_code != pch->RxFrameData[1]) {
        return (MODBUS_ERR_FC);
    }

    reg_addr_hi = MBM_TX_FRAME_FC06_ADDR_HI;                  /* Validate register address                   */
    reg_addr_lo = MBM_TX_FRAME_FC06_ADDR_LO;
    if ((reg_addr_hi != pch->RxFrameData[2]) || (reg_addr_lo != pch->RxFrameData[3])) {
        return (MODBUS_ERR_REG_ADDR);
    }

    return (MODBUS_ERR_NONE);
}
#endif

/*
*********************************************************************************************************
*                                             MBM_RegWrN_Resp()
*
* Description : Checks the slave's response to a request to write a register.
*
* Argument(s) : pch             A pointer to the channel that the message was received on.
*
* Return(s)   : MODBUS_ERR_NONE          If the function was sucessful.
*               MODBUS_ERR_SLAVE_ADDR    If the transmitted slave address doesn't correspond to the received slave address
*               MODBUS_ERR_FC            If the transmitted function code doesn't correspond to the received function code
*               MODBUS_ERR_BYTE_COUNT    If the expected number of bytes to receive doesn't correspond to the number of bytes received
*
* Caller(s)   : MBM_FC16_HoldingRegWrN(),
*               MBM_FC16_HoldingRegWrNFP().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC16_EN == DEF_ENABLED)
static  uint16_t  MBM_RegWrN_Resp (elab_mb_channel_t *pch)
{
    uint8_t   slave_addr;
    uint8_t   fnct_code;
    uint8_t   reg_addr_hi;
    uint8_t   reg_addr_lo;
    uint8_t   nbr_reg_hi;
    uint8_t   nbr_reg_lo;



    slave_addr = MBM_TX_FRAME_SLAVE_ADDR;                     /* Validate slave address                      */
    if (slave_addr != pch->RxFrameData[0]) {
        return (MODBUS_ERR_SLAVE_ADDR);
    }

    fnct_code  = MBM_TX_FRAME_FC;                             /* Validate function code                      */
    if (fnct_code != pch->RxFrameData[1]) {
        return (MODBUS_ERR_FC);
    }

    reg_addr_hi = MBM_TX_FRAME_FC16_ADDR_HI;                  /* Validate register address                   */
    reg_addr_lo = MBM_TX_FRAME_FC16_ADDR_LO;
    if ((reg_addr_hi != pch->RxFrameData[2]) || (reg_addr_lo != pch->RxFrameData[3])) {
        return (MODBUS_ERR_REG_ADDR);
    }

    nbr_reg_hi = MBM_TX_FRAME_FC16_NBR_REGS_HI;               /* Validate number of registers                */
    nbr_reg_lo = MBM_TX_FRAME_FC16_NBR_REGS_LO;
    if ((nbr_reg_hi != pch->RxFrameData[4]) || (nbr_reg_lo != pch->RxFrameData[5])) {
        return (MODBUS_ERR_NBR_REG);
    }

    return (MODBUS_ERR_NONE);
}
#endif

/*
*********************************************************************************************************
*                                       MBM_Diag_Resp()
*
* Description : Checks the response from a slave unit that was sent a diagnostic LOOPBACK function request.
*               The request contains various diagnostic codes that perform specific actions.  This function
*               processes the response for the sent diagnostic code.  If a request was sent to read any of
*               the communications registers then the information is placed at the location pointed by 'pval'
*
* Argument(s) : pch             A pointer to the channel that the message was received on.
*
*               pval            A pointer to where the loopback value is placed
*
* Return(s)   : MODBUS_ERR_NONE    If the function was sucessful.
*               err_code           Otherwise.
*
* Caller(s)   : MBM_FC08_Diag().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC08_EN == DEF_ENABLED)
static  uint16_t  MBM_Diag_Resp (elab_mb_channel_t *pch, uint16_t *pval)
{
    uint8_t   slave_addr;
    uint8_t   fnct_code;
    uint8_t   sub_code_hi;
    uint8_t   sub_code_lo;
    uint16_t   sub_code;
    uint8_t   data_hi;
    uint8_t   data_lo;



    slave_addr = MBM_TX_FRAME_SLAVE_ADDR;              /* Validate slave address                                   */
    if (slave_addr != pch->RxFrameData[0]) {
        return (MODBUS_ERR_SLAVE_ADDR);
    }

    fnct_code  = MBM_TX_FRAME_FC;                      /* Validate function code                                   */
    if (fnct_code != pch->RxFrameData[1]) {
        return (MODBUS_ERR_FC);
    }

    sub_code_hi = MBM_TX_FRAME_FC08_FNCT_HI;           /* Validate sub-function                                    */
    sub_code_lo = MBM_TX_FRAME_FC08_FNCT_LO;
    if ((sub_code_hi != pch->RxFrameData[2]) || (sub_code_lo != pch->RxFrameData[3])) {
        return (MODBUS_ERR_SUB_FNCT);
    }
    sub_code = ((uint16_t)sub_code_hi << 8) + (uint16_t)sub_code_lo;
    data_hi  = MBM_TX_FRAME_FC08_FNCT_DATA_HI;
    data_lo  = MBM_TX_FRAME_FC08_FNCT_DATA_LO;

    switch (sub_code) {
        case MODBUS_FC08_LOOPBACK_QUERY:               /* Return Query function code - no need to do anything.     */
        case MODBUS_FC08_LOOPBACK_CLR_CTR:
             if ((data_hi != pch->RxFrameData[4]) || (data_lo != pch->RxFrameData[5])) {
                 *pval = 0;
                 return (MODBUS_ERR_DIAG);
             } else {
                 *pval = ((uint16_t)pch->RxFrameData[4] << 8) + (uint16_t)pch->RxFrameData[5];
                 return (MODBUS_ERR_NONE);
             }

        case MODBUS_FC08_LOOPBACK_BUS_MSG_CTR:         /* Return the message count in diag information field.      */
        case MODBUS_FC08_LOOPBACK_BUS_CRC_CTR:         /* Return the CRC error count in diag information field.    */
        case MODBUS_FC08_LOOPBACK_BUS_EXCEPT_CTR:      /* Return exception count in diag information field.        */
        case MODBUS_FC08_LOOPBACK_SLAVE_MSG_CTR:       /* Return slave message count in diag information field.    */
        case MODBUS_FC08_LOOPBACK_SLAVE_NO_RESP_CTR:   /* Return no response count in diag information field.      */
             *pval = ((uint16_t)pch->RxFrameData[4] << 8) + (uint16_t)pch->RxFrameData[5];
             return (MODBUS_ERR_NONE);

        default:
             return (MODBUS_ERR_DIAG);
    }
}
#endif


/*
*********************************************************************************************************
*                                             MBM_RxReply()
*
* Description : Receive a reply from a Slave
*
* Argument(s) : ch       Specifies the Modbus channel on which the reply is coming from
*
* Return(s)   : none.
*
* Caller(s)   : MBM_Fxx  Modbus Master Functions.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  bool  MBM_RxReply (elab_mb_channel_t *pch)
{
    bool ok;

    ok = MODBUS_FALSE;
    if (pch != (elab_mb_channel_t *)0) {
#if (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
        if (pch->Mode == MODBUS_MODE_ASCII) {
            ok = MB_ASCII_Rx(pch);
        }
#endif

#if (MODBUS_CFG_RTU_EN != 0)
        if (pch->Mode == MODBUS_MODE_RTU) {
            ok = MB_RTU_Rx(pch);
        }
#endif
    }
    return (ok);
}


/*
*********************************************************************************************************
*                                              MBM_TxCmd()
*
* Description : Send a command to a Slave
*
* Argument(s) : ch       Specifies the Modbus channel on which the command will be sent
*
* Return(s)   : none.
*
* Caller(s)   : MBM_Fxx  Modbus Master Functions.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  MBM_TxCmd (elab_mb_channel_t *pch)
{
    if (pch != (elab_mb_channel_t *)0) {
#if (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
        if (pch->Mode == MODBUS_MODE_ASCII) {
            MB_ASCII_Tx(pch);
        }
#endif

#if (MODBUS_CFG_RTU_EN != 0)
        if (pch->Mode == MODBUS_MODE_RTU) {
            MB_RTU_Tx(pch);
        }
#endif
    }
}

#endif
