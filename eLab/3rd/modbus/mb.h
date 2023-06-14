/*
********************************************************************************
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
********************************************************************************
*/

/*
********************************************************************************
*                                       uC/MODBUS Header File
*
* Filename : mb.h
* Version  : V2.14.00
********************************************************************************
*/

 
/*
********************************************************************************
*                                               MODULE
*
* Note(s) : (1) This main modbus header file is protected from multiple pre-processor
*               inclusion through use of the modbus module present pre-processor macro definition.
*
********************************************************************************
*/

#ifndef  MODBUS_MODULE_PRESENT                                  /* See Note #1.                                         */
#define  MODBUS_MODULE_PRESENT

/*
********************************************************************************
*                                               EXTERNS
********************************************************************************
*/

#ifdef    MB_MODULE
#define   MB_EXT
#else
#define   MB_EXT  extern
#endif


#include  "mb_cfg.h"
#include  "mb_def.h"
#include "../../RTOS/cmsis_os.h"

/*
********************************************************************************
*                                               DATA TYPES
********************************************************************************
*/

typedef struct elab_mb_channel_cb
{
    bool (* coil_read)(uint16_t coil, uint16_t *perr);
    void (* coil_write)(uint16_t coil, bool coil_val, uint16_t *perr);
    bool (* di_read)(uint16_t di, uint16_t *perr);
    uint16_t (* in_reg_read)(uint16_t reg, uint16_t *perr);
    float (* in_reg_read_fp)(uint16_t reg, uint16_t *perr);

    uint16_t (* holding_reg_read)(uint16_t reg, uint16_t *perr);
    float (* holding_reg_read_fp)(uint16_t reg, uint16_t *perr);
    void (* holding_reg_write)(uint16_t reg, uint16_t reg_val_16, uint16_t *perr);
    void (* holding_reg_write_fp)(uint16_t reg, float reg_val_fp, uint16_t *perr);

    uint16_t (* file_read)(uint16_t file_nbr, uint16_t record_nbr, uint16_t ix,
                            uint8_t record_len, uint16_t *perr);

    uint16_t (* file_write)(uint16_t file_nbr, uint16_t record_nbr, uint16_t ix,
                            uint8_t record_len, uint16_t value, uint16_t *perr);
} elab_mb_channel_cb_t;

typedef  struct  elab_mb_channel_t {
    uint8_t       Ch;                               /* Channel number                                                   */
    bool write_en;                             /* Indicates whether MODBUS writes are enabled for the channel      */
    uint32_t       WrCtr;                            /* Incremented each time a write command is performed               */

    uint8_t       NodeAddr;                         /* Modbus node address of the channel                               */

    uint8_t       PortNbr;                          /* UART port number                                                 */
    uint32_t       baud_rate;                         /* Baud Rate                                                        */
    uint8_t       Parity;                           /* UART's parity settings (MODBUS_PARITY_NONE, _ODD or _EVEN)       */
    uint8_t       Bits;                             /* UART's number of bits (7 or 8)                                   */
    uint8_t       Stops;                            /* UART's number of stop bits (1 or 2)                              */

    uint8_t       Mode;                             /* Modbus mode: MODBUS_MODE_ASCII or MODBUS_MODE_RTU                */

    uint8_t m_or_s;                      /* Slave when set to MODBUS_SLAVE, Master when set to MODBUS_MASTER */

    uint16_t       Err;                              /* Internal code to indicate the source of MBS_ErrRespSet()         */

#if (MODBUS_CFG_RTU_EN != 0)
    uint16_t       RTU_TimeoutCnts;                  /* Counts to reload in .RTU_TimeoutCtr when byte received           */
    uint16_t       RTU_TimeoutCtr;                   /* Counts left before RTU timer times out for the channel           */
    bool      RTU_TimeoutEn;                    /* Enable (when TRUE) or Disable (when FALSE) RTU timer             */
#endif

#if (MODBUS_CFG_FC08_EN == DEF_ENABLED)
    uint16_t       StatMsgCtr;                       /* Statistics                                                       */
    uint16_t       StatCRCErrCtr;
    uint16_t       StatExceptCtr;
    uint16_t       StatSlaveMsgCtr;
    uint16_t       StatNoRespCtr;
#endif

    uint32_t       RxTimeout;                        /* Amount of time Master is willing to wait for response from slave */

    uint32_t       RxCtr;                            /* Incremented every time a character is received                   */
    uint16_t       RxBufByteCtr;                     /* Number of bytes received or to send                              */
    uint8_t      *RxBufPtr;                         /* Pointer to current position in buffer                            */
    uint8_t       RxBuf[MODBUS_CFG_BUF_SIZE];       /* Storage of received characters or characters to send             */

    uint32_t       TxCtr;                            /* Incremented every time a character is transmitted                */
    uint16_t       TxBufByteCtr;                     /* Number of bytes received or to send                              */
    uint8_t      *TxBufPtr;                         /* Pointer to current position in buffer                            */
    uint8_t       TxBuf[MODBUS_CFG_BUF_SIZE];       /* Storage of received characters or characters to send             */

    uint8_t       RxFrameData[MODBUS_CFG_BUF_SIZE]; /* Additional data for function requested.                          */
    uint16_t       RxFrameNDataBytes;                /* Number of bytes in the data field.                               */
    uint16_t       RxFrameCRC;                       /* Error check value (LRC or CRC-16).                               */
    uint16_t       RxFrameCRC_Calc;                  /* Error check value computed from packet received                  */

    uint8_t       TxFrameData[MODBUS_CFG_BUF_SIZE]; /* Additional data for function requested.                          */
    uint16_t       TxFrameNDataBytes;                /* Number of bytes in the data field.                               */
    uint16_t       TxFrameCRC;                       /* Error check value (LRC or CRC-16).                               */

    elab_mb_channel_cb_t cb;
    osMutexId_t mutex;
    bool no_ack;
    uint32_t no_ack_timeout_ms;
} elab_mb_channel_t;

 
/*
********************************************************************************
*                           GLOBAL VARIABLES
********************************************************************************
*/

#if (MODBUS_CFG_RTU_EN != 0)
extern uint16_t MB_RTU_Freq;                  /* Frequency at which RTU timer is running                          */
extern uint32_t MB_RTU_TmrCtr;                /* Incremented every Modbus RTU timer interrupt                     */
#endif

// extern uint8_t MB_ChCtr;                     /* Modbus channel counter (0..MODBUS_MAX_CH)                        */
extern elab_mb_channel_t modbus_channel_table[MODBUS_CFG_MAX_CH];  /* Modbus channels                                                  */

/*
********************************************************************************
*                                           GLOBAL VARIABLES
********************************************************************************
*/

extern  uint32_t  const  MB_TotalRAMSize;
extern  uint16_t  const  MB_ChSize;

/*
********************************************************************************
*                                  MODBUS INTERFACE FUNCTION PROTOTYPES
*                                   (MB.C)
********************************************************************************
*/

void elab_mb_init(uint32_t freq);
void elab_mb_exit(void);

elab_mb_channel_t *elab_mb_config_channel(
                        uint8_t node_addr, uint8_t master_slave,
                        uint32_t rx_timeout, uint8_t modbus_mode,
                        uint8_t port_nbr, uint32_t baud, uint8_t bits,
                        uint8_t parity, uint8_t stops,
                        uint8_t wr_en);
/* Add by GouGe.*/
void elab_mb_slave_set_cb(elab_mb_channel_t *pch, elab_mb_channel_cb_t *cb);
void elab_mb_master_set_timeout(elab_mb_channel_t *pch, uint32_t timeout);
void elab_mb_set_mode(elab_mb_channel_t *pch, uint8_t master_slave, uint8_t mode);
void elab_mb_set_node_addr(elab_mb_channel_t *pch, uint8_t addr);
void elab_mb_set_write_en(elab_mb_channel_t *pch, uint8_t wr_en);
void elab_mb_channel_map_port(elab_mb_channel_t *pch, uint8_t port_nbr);

#if (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
void          MB_ASCII_RxByte  (elab_mb_channel_t   *pch,
                                         uint8_t   rx_byte);
#endif

#if (MODBUS_CFG_RTU_EN != 0)
void MB_RTU_RxByte(elab_mb_channel_t *pch, uint8_t rx_byte);

void MB_RTU_TmrReset(elab_mb_channel_t *pch);       /* Resets the Frame Sync timer.                                 */

void MB_RTU_TmrResetAll(void);                   /* Resets all the RTU timers                                    */

void MB_RTU_TmrUpdate(void);
#endif

void MB_RxByte(elab_mb_channel_t *pch, uint8_t rx_byte);

void MB_RxTask(elab_mb_channel_t*pch);

void MB_Tx(elab_mb_channel_t   *pch);

void          MB_TxByte     (elab_mb_channel_t   *pch);

#if (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
bool  MB_ASCII_Rx    (elab_mb_channel_t   *pch);
void         MB_ASCII_Tx    (elab_mb_channel_t   *pch);
#endif


#if (MODBUS_CFG_RTU_EN != 0)
bool MB_RTU_Rx(elab_mb_channel_t *pch);
void MB_RTU_Tx(elab_mb_channel_t *pch);
#endif

/*
********************************************************************************
*                   RTOS INTERFACE FUNCTION PROTOTYPES
*           (defined in mb_os.c)
********************************************************************************
*/

void elab_mb_os_init(void);
void elab_mb_os_exit(void);
void elab_mb_os_rx_signal(elab_mb_channel_t *pch);
void elab_mb_os_rx_wait(elab_mb_channel_t *pch, uint16_t *perr);

/*
********************************************************************************
*           COMMON MODBUS ASCII INTERFACE FUNCTION PROTOTYPES
*           (defined in mb_util.c)
********************************************************************************
*/

#if (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
uint8_t *MB_ASCII_BinToHex(uint8_t value, uint8_t *pbuf);
uint8_t MB_ASCII_HexToBin(uint8_t *phex);
uint8_t MB_ASCII_RxCalcLRC(elab_mb_channel_t *pch);
uint8_t MB_ASCII_TxCalcLRC(elab_mb_channel_t *pch, uint16_t tx_bytes);
#endif

/*
********************************************************************************
*                             COMMON MODBUS RTU INTERFACE FUNCTION PROTOTYPES
*                           (defined in mb_util.C)
********************************************************************************
*/

#if (MODBUS_CFG_RTU_EN != 0)
uint16_t MB_RTU_CalcCRC(elab_mb_channel_t  *pch);

uint16_t MB_RTU_TxCalcCRC(elab_mb_channel_t  *pch);

uint16_t MB_RTU_RxCalcCRC(elab_mb_channel_t  *pch);
#endif

/*
********************************************************************************
*                                    INTERFACE TO APPLICATION DATA
*                           (defined in mb_data.C)
********************************************************************************
*/

#if (MODBUS_CFG_FC01_EN == DEF_ENABLED)
bool MB_CoilRd(uint16_t coil, uint16_t *perr);
#endif

#if (MODBUS_CFG_FC05_EN == DEF_ENABLED)
void MB_CoilWr(uint16_t coil, bool coil_val, uint16_t *perr);
#endif

#if (MODBUS_CFG_FC02_EN == DEF_ENABLED)
bool MB_DIRd(uint16_t di, uint16_t *perr);
#endif

#if (MODBUS_CFG_FC04_EN == DEF_ENABLED)
uint16_t MB_InRegRd(uint16_t reg, uint16_t *perr);
float MB_InRegRdFP(uint16_t reg, uint16_t *perr);
#endif

#if (MODBUS_CFG_FC03_EN == DEF_ENABLED)
uint16_t MB_HoldingRegRd(uint16_t reg, uint16_t *perr);
float MB_HoldingRegRdFP(uint16_t reg, uint16_t *perr);
#endif

#if (MODBUS_CFG_FC06_EN == DEF_ENABLED) || (MODBUS_CFG_FC16_EN == DEF_ENABLED)
void MB_HoldingRegWr(uint16_t reg, uint16_t reg_val_16, uint16_t *perr);
void MB_HoldingRegWrFP(uint16_t reg, float reg_val_fp, uint16_t *perr);
#endif

#if (MODBUS_CFG_FC20_EN == DEF_ENABLED)
uint16_t elab_mb_file_read(uint16_t file_nbr,
                            uint16_t record_nbr,
                            uint16_t ix,
                            uint8_t record_len,
                            uint16_t *perr);
#endif

#if (MODBUS_CFG_FC21_EN == DEF_ENABLED)
void elab_mb_file_write(uint16_t file_nbr, uint16_t record_nbr,
                        uint16_t ix, uint8_t record_len,
                        uint16_t value, uint16_t *perr);
#endif

/*
********************************************************************************
*                           BSP FUNCTION PROTOTYPES
********************************************************************************
*/

void MB_CommExit(void);                   /* Exit       Modbus Communications                             */

void MB_CommPortCfg(elab_mb_channel_t *pch,
                                         uint8_t   port_nbr,
                                         uint32_t   baud,
                                         uint8_t   bits,
                                         uint8_t   parity,
                                         uint8_t   stops);


void MB_CommRxIntEn(elab_mb_channel_t *pch);           /* Enable  Rx interrupts                                        */

void MB_CommRxIntDis(elab_mb_channel_t   *pch);           /* Disable Rx interrupts                                        */

void MB_CommTx1(elab_mb_channel_t   *pch,
                                         uint8_t   c);

void MB_CommTxIntEn(elab_mb_channel_t   *pch);           /* Enable  Tx interrupts                                        */

void MB_CommTxIntDis(elab_mb_channel_t   *pch);           /* Disable Tx interrupts                                        */


#if (MODBUS_CFG_RTU_EN != 0)
void elab_mb_rtu_timer_init (void);                       /* Initialize the timer used for RTU framing                    */
void elab_mb_rtu_timer_exit (void);
void MB_RTU_TmrISR_Handler(void);
#endif

/*
********************************************************************************
*                                            MODBUS SLAVE
*                                      GLOBAL FUNCTION PROTOTYPES
********************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
bool MBS_FCxx_Handler(elab_mb_channel_t *pch);
void MBS_RxTask(elab_mb_channel_t *pch);

#if (MODBUS_CFG_FC08_EN == DEF_ENABLED)
void MBS_StatInit(elab_mb_channel_t *pch);
#endif
#endif

/*
********************************************************************************
*                                            MODBUS MASTER
*                                      GLOBAL FUNCTION PROTOTYPES
********************************************************************************
*/

#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)

#if (MODBUS_CFG_FC01_EN == DEF_ENABLED)
uint16_t MBM_FC01_CoilRd(elab_mb_channel_t *pch, uint8_t slave_addr,
                            uint16_t start_addr, uint8_t *p_coil_tbl,
                            uint16_t nbr_coils);
#endif

#if (MODBUS_CFG_FC02_EN == DEF_ENABLED)
uint16_t MBM_FC02_DIRd(elab_mb_channel_t *pch, uint8_t slave_node,
                                      uint16_t   slave_addr,
                                      uint8_t  *p_di_tbl,
                                      uint16_t   nbr_di);
#endif

#if (MODBUS_CFG_FC03_EN == DEF_ENABLED)
uint16_t MBM_FC03_HoldingRegRd(elab_mb_channel_t *pch,
                                      uint8_t slave_node,
                                      uint16_t slave_addr,
                                      uint16_t *p_reg_tbl,
                                      uint16_t nbr_regs);
#endif

#if (MODBUS_CFG_FC03_EN == DEF_ENABLED) && \
 (MODBUS_CFG_FP_EN   == DEF_ENABLED)
uint16_t  MBM_FC03_HoldingRegRdFP  (elab_mb_channel_t   *pch,
                                      uint8_t   slave_node,
                                      uint16_t   slave_addr,
                                      float    *p_reg_tbl,
                                      uint16_t   nbr_regs);
#endif

#if (MODBUS_CFG_FC04_EN == DEF_ENABLED)
uint16_t  MBM_FC04_InRegRd(elab_mb_channel_t   *pch,
                                      uint8_t   slave_node,
                                      uint16_t   slave_addr,
                                      uint16_t  *p_reg_tbl,
                                      uint16_t   nbr_regs);
#endif

#if (MODBUS_CFG_FC05_EN == DEF_ENABLED)
uint16_t  elab_mbm_fc05_wirte_coil (elab_mb_channel_t   *pch,
                                      uint8_t   slave_node,
                                      uint16_t   slave_addr,
                                      bool  coil_val);
#endif

#if (MODBUS_CFG_FC06_EN == DEF_ENABLED)
uint16_t  MBM_FC06_HoldingRegWr (elab_mb_channel_t   *pch,
                                      uint8_t   slave_node,
                                      uint16_t   slave_addr,
                                      uint16_t   reg_val);

uint16_t  MBM_FC06_HoldingRegWr_NoAck(elab_mb_channel_t   *pch,
                                      uint8_t   slave_node,
                                      uint16_t   slave_addr,
                                      uint16_t   reg_val,
                                      uint32_t timeout_ms);
#endif

#if (MODBUS_CFG_FC06_EN == DEF_ENABLED) && (MODBUS_CFG_FP_EN   == DEF_ENABLED)
uint16_t  MBM_FC06_HoldingRegWrFP  (elab_mb_channel_t   *pch,
                                      uint8_t   slave_node,
                                      uint16_t   slave_addr,
                                      float     reg_val_fp);
#endif

#if (MODBUS_CFG_FC08_EN == DEF_ENABLED)
uint16_t  MBM_FC08_Diag(elab_mb_channel_t   *pch,
                                      uint8_t   slave_node,
                                      uint16_t   fnct,
                                      uint16_t   fnct_data,
                                      uint16_t  *pval);
#endif

#if (MODBUS_CFG_FC15_EN == DEF_ENABLED)
uint16_t MBM_FC15_CoilWr(elab_mb_channel_t *pch, uint8_t slave_node,
                                      uint16_t slave_addr,
                                      uint8_t *p_coil_tbl,
                                      uint16_t nbr_coils);
#endif

#if (MODBUS_CFG_FC16_EN == DEF_ENABLED)
uint16_t MBM_FC16_HoldingRegWrN(elab_mb_channel_t *pch,
                                      uint8_t   slave_node,
                                      uint16_t   slave_addr,
                                      uint16_t  *p_reg_tbl,
                                      uint16_t   nbr_regs);
#endif

#if (MODBUS_CFG_FC16_EN == DEF_ENABLED) && \
 (MODBUS_CFG_FP_EN   == DEF_ENABLED)
uint16_t  MBM_FC16_HoldingRegWrNFP (elab_mb_channel_t   *pch,
                                      uint8_t   slave_node,
                                      uint16_t   slave_addr,
                                      float    *p_reg_tbl,
                                      uint16_t   nbr_regs);
#endif

#endif
/*
********************************************************************************
*                                            SYMBOL ERRORS
********************************************************************************
*/

#ifndef  MODBUS_CFG_MASTER_EN
#error  "MODBUS_CFG_MASTER_Enot #defined"
#error  "... Defines wheteher your product will support Modbus Master"
#endif

#ifndef  MODBUS_CFG_SLAVE_EN
#error  "MODBUS_CFG_SLAVE_EN not #defined."
#error  "... Defines wheteher your product will support Modbus Slave"
#endif

#ifndef  MODBUS_CFG_MAX_CH
#error  "MODBUS_CFG_MAX_CH  not #defined."
#error  "... Defines the number of Modbus ports supported. Should be 1 to N."
#endif

#ifndef  MODBUS_CFG_MAX_CH
#error  "MODBUS_CFG_MAX_CH  not #defined."
#error  "... Defines the number of Modbus ports supported. Should be 1 to N."
#endif


#ifndef  MODBUS_CFG_MAX_CH
#error  "MODBUS_CFG_MAX_CH  not #defined."
#error  "... Defines the number of Modbus ports supported. Should be 1 to N."
#endif

#ifndef  MODBUS_CFG_ASCII_EN
#error  "MODBUS_CFG_ASCII_EN not #defined"
#error  "... Defines whether your product will support Modbus ASCII."
#endif

#ifndef  MODBUS_CFG_RTU_EN
#error  "MODBUS_CFG_RTU_EN  not #defined."
#error  "... Defines whether your product will support Modbus RTU."
#endif

#ifndef  MODBUS_CFG_FP_EN
#error  "MODBUS_CFG_FP_EN   not #defined."
#error  "... Defines whether your product will support Daniels Flow Meter Floating-Point extensions."
#endif

#ifndef  MODBUS_CFG_FP_START_IX
#error  "MODBUS_CFG_FP_START_not #defined"
#error  "... Defines the starting register number for floating-point registers."
#endif

#ifndef  MODBUS_CFG_FC01_EN
#error  "MODBUS_CFG_FC01_EN not #defined."
#endif

#ifndef  MODBUS_CFG_FC02_EN
#error  "MODBUS_CFG_FC02_EN not #defined."
#endif

#ifndef  MODBUS_CFG_FC03_EN
#error  "MODBUS_CFG_FC03_EN not #defined."
#endif

#ifndef  MODBUS_CFG_FC04_EN
#error  "MODBUS_CFG_FC04_EN not #defined."
#endif

#ifndef  MODBUS_CFG_FC05_EN
#error  "MODBUS_CFG_FC05_EN not #defined."
#endif

#ifndef  MODBUS_CFG_FC06_EN
#error  "MODBUS_CFG_FC06_EN not #defined."
#endif

#ifndef  MODBUS_CFG_FC08_EN
#error  "MODBUS_CFG_FC08_EN not #defined."
#endif

#ifndef  MODBUS_CFG_FC15_EN
#error  "MODBUS_CFG_FC15_EN not #defined."
#endif

#ifndef  MODBUS_CFG_FC16_EN
#error  "MODBUS_CFG_FC16_EN not #defined."
#endif

#ifndef  MODBUS_CFG_FC20_EN
#error  "MODBUS_CFG_FC20_EN not #defined."
#endif

#ifndef  MODBUS_CFG_FC21_EN
#error  "MODBUS_CFG_FC21_EN not #defined."
#endif

#endif

