
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#define MB_OS_MODULE

#include "mb.h"
#include "cmsis_os.h"
#include "elab_log.h"
#include "elab_assert.h"

ELAB_TAG("modbus_os");

/* Private variables ---------------------------------------------------------*/
#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
static osSemaphoreId_t mb_sem_rx[MODBUS_CFG_MAX_CH];
#endif

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
static osMessageQueueId_t mb_mq = NULL;
static osThreadId_t mb_thread = NULL;

static const osThreadAttr_t mb_thread_rx_attr =
{
    .name = "mb_thread_rx",
    .attr_bits = osThreadDetached,
    .priority = osPriorityRealtime,
    .stack_size = 2048,
};

#endif

/* Private function prototypes -----------------------------------------------*/
#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
static void elab_mb_os_init_master(void);
static void elab_mb_os_exit_master(void);
#endif

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
static void elab_mb_os_init_slave(void);
static void elab_mb_os_exit_slave(void);

static void  _entry_slave_rx(void *paras);
#endif

/* Exported functions --------------------------------------------------------*/
/**
 * @brief  This function initializes the RTOS interface. This function creates
 *         the following:
 *              (1) A message queue to signal the reception of a packet.
 *              (2) A task that waits for packets to be received.
 * @retval None.
 */
void elab_mb_os_init(void)
{
#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
    elab_mb_os_init_master();
#endif

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
    elab_mb_os_init_slave();
#endif
}

/**
 * @brief  This function initializes and creates the kernel objects needed for
 *         Modbus Master.
 * @retval None.
 */
#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
static void elab_mb_os_init_master(void)
{
    /* Create a semaphore for each channel   */
    for (uint32_t i = 0; i < MODBUS_CFG_MAX_CH; i++)
    {
        mb_sem_rx[i] = osSemaphoreNew(1, 0, NULL);
        elab_assert(mb_sem_rx[i] != NULL);
    }
}
#endif

/**
 * @brief  This function initializes and creates the kernel objects needed for
 *         Modbus Master.
 * @retval None.
 */
#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
static void elab_mb_os_init_slave(void)
{
    /* Create a message queue for all the slave channels. */
    mb_mq = osMessageQueueNew(32, sizeof(elab_mb_channel_t *), NULL);
    elab_assert(mb_mq != NULL);

    /* Create a rx thread for all the slave channels. */
    mb_thread = osThreadNew(_entry_slave_rx, NULL, &mb_thread_rx_attr);
    elab_assert(mb_thread != NULL);
}
#endif

/**
 * @brief  This function is called to terminate the RTOS interface for Modbus
 *         channels. We delete the following uC/OS-II objects:
 *              (1) An event flag group to signal the reception of a packet.
 *              (2) A task that waits for packets to be received.
 * @retval None.
 */
void elab_mb_os_exit(void)
{
#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
    elab_mb_os_exit_master();
#endif

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
    elab_mb_os_exit_slave();
#endif
}

/**
 * @brief  This function is called to terminate the RTOS interface for Modbus 
 *         Master channels. The following objects are deleted.
 *              (1) An event flag group to signal the reception of a packet.
 * @retval None.
 */
#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
static void elab_mb_os_exit_master (void)
{
    osStatus_t status;

    /* Delete semaphore for each channel     */
    for (uint32_t i = 0; i < MODBUS_CFG_MAX_CH; i++)
    {
        status = osSemaphoreDelete(mb_sem_rx[i]);
        elab_assert(osOK == status);
    }
}
#endif

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
static void elab_mb_os_exit_slave(void)
{
    /* Delete the message queue. */
    osStatus_t ret = osMessageQueueDelete(mb_mq);
    elab_assert(ret == osOK);
    mb_mq = NULL;
}
#endif

/**
 * @brief  This function signals the reception of a packet either from the Rx 
 *         ISR(s) or the RTU timeout timer(s) to indicate that a received packet
 *         needs to be processed.
 * @param  pch     specifies the Modbus channel data structure in which a packet
 *                 was received.
 * @retval None.
 */
void elab_mb_os_rx_signal(elab_mb_channel_t *pch)
{
    osStatus_t ret;

    if (pch != (elab_mb_channel_t *)0)
    {
        switch (pch->m_or_s)
        {
#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
            case MODBUS_MASTER:
                ret = osSemaphoreRelease(mb_sem_rx[pch->Ch]);
                elab_assert(osOK == ret);
                break;
#endif

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
            case MODBUS_SLAVE:
            default:
                ret = osMessageQueuePut(mb_mq, &pch, 0, osWaitForever);
                elab_assert(ret == osOK);
                break;
#endif
        }
    }
}

/**
 * @brief  This function waits for a response from a slave.
 * @param  pch     specifies the Modbus channel data structure to wait on.
 * @param  perr    is a pointer to a variable that will receive an error code.
 *                 Possible errors are:
 *                      MODBUS_ERR_NONE  the call was successful and a packet
 *                                       was received
 *                      MODBUS_ERR_TIMED_OUT  a packet was not received within
 *                                            the specified timeout
 *                      MODBUS_ERR_NOT_MASTER  the channel is not a Master
 *                      MODBUS_ERR_INVALID     an invalid error was detected
 * @retval None.
 */
void elab_mb_os_rx_wait(elab_mb_channel_t *pch, uint16_t *perr)
{
#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
    osStatus_t status;

    if (pch != (elab_mb_channel_t *)0)
    {
        if (pch->m_or_s == MODBUS_MASTER)
        {
            status = osSemaphoreAcquire(mb_sem_rx[pch->Ch], pch->RxTimeout);
            switch (status)
            {
                case osErrorTimeout:
                    *perr = MODBUS_ERR_TIMED_OUT;
                    break;

                case osOK:
                    *perr = MODBUS_ERR_NONE;
                    break;

                default:
                    *perr = MODBUS_ERR_INVALID;
                    break;
            }
        }
        else
        {
            *perr = MODBUS_ERR_NOT_MASTER;
        }
    }
    else
    {
        *perr = MODBUS_ERR_NULLPTR;
    }
#else
    elab_assert(false);
#endif
}

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
static void  _entry_slave_rx(void *paras)
{
    osStatus_t ret = osOK;
    elab_mb_channel_t *pch = NULL;

    (void)paras;

    while (1)
    {
        pch = NULL;
        ret = osMessageQueueGet(mb_mq, &pch, NULL, osWaitForever);

        elab_assert(ret == osOK);
        elab_assert(pch != NULL);

        MB_RxTask(pch);
    }

    osThreadExit();
}
#endif

/* ----------------------------- end of file -------------------------------- */
