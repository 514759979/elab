/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_spi.h"
#include "elab_assert.h"
#include "elab_log.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("Edf_SPI");

/* Private variables ---------------------------------------------------------*/
static const elab_dev_ops_t spi_ops =
{
    .enable = NULL,
    .read = NULL,
    .write = NULL,
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
    .poll = NULL,
#endif
};

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  SPI bus device initialization and registering.
  * @param  bus  The SPI bus handle
  * @param  name Bus name
  * @retval None.
  */
void elab_spi_bus_register(elab_spi_bus_t *bus,
                            const char *name, const elab_spi_bus_ops_t *ops,
                            void *user_data)
{
    assert(bus != NULL);
    assert(name != NULL);
    assert(ops != NULL);

    elab_device_t *device = &bus->super;

    /* initialize device interface. */
    device->ops = &spi_ops;
    bus->ops = ops;
    bus->super.user_data = user_data;
    bus->config_owner.data_width = UINT8_MAX;
    bus->config_owner.mode = UINT8_MAX;
    bus->config_owner.max_hz = UINT32_MAX;

    /* Initialize mutex mutex */
    const osMutexAttr_t mutex_attr =
    {
        "spi_bus_mutex",
        osMutexPrioInherit,
        NULL,
        0U
    };
    bus->mutex = osMutexNew(&mutex_attr);
    assert(NULL != bus->mutex);

    /* register to device manager */
    elab_device_attr_t attr_spi_bus =
    {
        .name = name,
        .sole = false,
        .type = ELAB_DEVICE_SPI_BUS,
    };
    elab_device_register(device, &attr_spi_bus);
}

/**
  * @brief  SPI device init and register to elab Edf.
  * @param  dev  The pointer of elab Edf.
  * @param  name Device name
  * @retval See elab_err_t
  */
void elab_spi_register(elab_spi_t *device, const char *name,
                            const char *bus_name,
                            const elab_spi_ops_t *ops,
                            elab_spi_config_t config,
                            void *user_data)
{
    assert(device != NULL);
    assert(name != NULL);
    assert(bus_name != NULL);
    assert(ops != NULL);
 
    elab_spi_bus_t *bus = (elab_spi_bus_t *)elab_device_find(bus_name);
    assert(bus != NULL);

    device->bus = bus;
    device->config = config;
    device->ops = ops;
    device->super.user_data = user_data;

    /* Register to device manager. */
    elab_device_attr_t attr_spi =
    {
        .name = name,
        .sole = true,
        .type = ELAB_DEVICE_SPI,
    };
    elab_device_register(&device->super, &attr_spi);
}

/**
  * @brief  Send data then send data from SPI device.
  * @param  device  The pointer of EDF SPI dev
  * @param  buff1   The poingter send buf1
  * @param  size1   Sending size 1.
  * @param  buff2   The poingter send buf2
  * @param  size2   Sending size 2.
  * @retval See elab_err_t
  */
elab_err_t elab_spi_send_twice(elab_device_t *me,
                                const void *buff1, uint32_t size1,
                                const void *buff2, uint32_t size2)
{
    assert(me != NULL);
    assert(buff1 != NULL);
    assert(buff2 != NULL);
    assert(size1 != 0);
    assert(size1 != 0);

    elab_err_t ret = ELAB_OK;
    osStatus_t ret_os = osOK;
    elab_spi_t *spi = (elab_spi_t *)me;
    elab_spi_msg_t msg;

    assert(spi->bus != NULL);

    ret_os = osMutexAcquire(spi->bus->mutex, osWaitForever);
    assert(ret_os == osOK);

    /* Not the same config as current, re-configure SPI bus */
    if (memcmp(&spi->bus->config_owner,
                &spi->config, sizeof(elab_spi_config_t)) != 0)
    {
        /* not the same owner as current, re-configure SPI bus */
        ret = spi->bus->ops->config(spi, &spi->config);

        if (ret != ELAB_OK)
        {
            goto exit_release_mutex;
        }

        /* Set SPI bus's config */
        spi->bus->config_owner = spi->config;
    }

    /* send data1 */
    msg.buff_send = buff1;
    msg.buff_recv = NULL;
    msg.size = size1;

    ret = spi->bus->ops->xfer(spi, &msg);
    if (ret != ELAB_OK)
    {
        goto exit_release_mutex;
    }

    /* send data2 */
    msg.buff_send = buff2;
    msg.buff_recv = NULL;
    msg.size = size2;

    /* Enable the SPI device. */
    spi->ops->enable(spi, true);
    /* Transfer msg */
    ret = spi->bus->ops->xfer(spi, &msg);
    /* Disable the SPI device. */
    spi->ops->enable(spi, false);
    
exit_release_mutex:
    ret_os = osMutexRelease(spi->bus->mutex);
    assert(ret_os == osOK);
    (void)ret_os;

    return ret;
}

/**
  * @brief  send data then receive data from SPI device
  * @param  device      The pointer of EDF SPI dev
  * @param  send_buf    The poingter send buf
  * @param  send_length Send length
  * @param  recv_buf    The poingter rcv buf
  * @param  recv_length Rcv length
  * @retval See elab_err_t
  */
elab_err_t elab_spi_send_recv(elab_device_t *me,
                                const void *buff_send, uint32_t size_send,
                                void *buff_recv, uint32_t size_recv)
{
    assert(me != NULL);
    assert(buff_send != NULL);
    assert(buff_recv != NULL);
    assert(size_send != 0);
    assert(size_recv != 0);
    
    elab_spi_t *spi = (elab_spi_t *)me;
    assert(spi->bus != NULL);

    elab_err_t ret = ELAB_OK;
    osStatus_t ret_os = osOK;
    elab_spi_msg_t msg;

    ret_os = osMutexAcquire(spi->bus->mutex, osWaitForever);
    assert(ret_os == osOK);

    /* not the same config as current, re-configure SPI bus */
    if (memcmp(&spi->bus->config_owner,
                &spi->config, sizeof(elab_spi_config_t)) != 0)
    {
        /* not the same owner as current, re-configure SPI bus */
        ret = spi->bus->ops->config(spi, &spi->config);
        if (ret != ELAB_OK)
        {
            goto exit_release_mutex;
        }

        /* Set SPI bus's config */
        spi->bus->config_owner = spi->config;
    }

    /* send data */
    msg.buff_send = buff_send;
    msg.buff_recv = NULL;
    msg.size = size_send;

    ret = spi->bus->ops->xfer(spi, &msg);
    if (ret != ELAB_OK)
    {
        goto exit_release_mutex;
    }

    /* recv data */
    msg.buff_send = NULL;
    msg.buff_recv = buff_recv;
    msg.size = size_recv;

    /* Enable the SPI device. */
    spi->ops->enable(spi, true);
    /* Transfer msg */
    ret = spi->bus->ops->xfer(spi, &msg);
    /* Disable the SPI device. */
    spi->ops->enable(spi, false);

exit_release_mutex:
    ret_os = osMutexRelease(spi->bus->mutex);
    assert(ret_os == osOK);
    (void)ret_os;

    return ret;
}

/**
 * This function transmits data to SPI device.
 *
 * @param me    The EDF device handle.
 * @param msg   the message buffer to be transmitted.
 * @param num   the buffer to save received data from SPI device.
 *
 * @return See elab_err_t
 */
elab_err_t elab_spi_xfer_msg(elab_device_t *me, elab_spi_msg_t *msg, uint32_t num)
{
    assert(me != NULL);
    assert(msg != NULL);
    assert(num != 0);

    elab_err_t ret = ELAB_OK;
    osStatus_t ret_os = osOK;
    elab_spi_t *spi = (elab_spi_t *)me;

    assert(spi->bus != NULL);

    ret_os = osMutexAcquire(spi->bus->mutex, osWaitForever);
    assert(ret_os == osOK);

    /* not the same config as current, re-configure SPI bus */
    if (memcmp(&spi->bus->config_owner,
                &spi->config, sizeof(elab_spi_config_t)) != 0)
    {
        ret = spi->bus->ops->config(spi, &spi->config);
        if (ret != ELAB_OK)
        {
            goto exit_release_mutex;
        }

        /* set SPI bus owner */
        spi->bus->config_owner = spi->config;
    }

    for (uint32_t i = 0; i < num; i ++)
    {
        /* Enable the SPI device. */
        spi->ops->enable(spi, true);
        /* Transfer msg */
        ret = spi->bus->ops->xfer(spi, &msg[i]);
        /* Disable the SPI device. */
        spi->ops->enable(spi, false);
        if (ret != ELAB_OK)
        {
            goto exit_release_mutex;
        }
    }

exit_release_mutex:
    ret_os = osMutexRelease(spi->bus->mutex);
    assert(ret_os == osOK);
    (void)ret_os;

    return ret;
}

/**
 * This function transmits data to SPI device.
 *
 * @param me        The EDF device handle.
 * @param send_buf  the buffer to be transmitted to SPI device.
 * @param recv_buf  the buffer to save received data from SPI device.
 * @param size      the size of transmitted data.
 *
 * @return See elab_err_t
 */
elab_err_t elab_spi_xfer(elab_device_t *me,
                            const void *buff_send, void *buff_recv,
                            uint32_t size)
{
    assert(me != NULL);
    assert(size != 0);

    elab_err_t ret = ELAB_OK;
    osStatus_t ret_os = osOK;
    elab_spi_msg_t msg;
    elab_spi_t *spi = (elab_spi_t *)me;

    assert(spi->bus != NULL);

    ret_os = osMutexAcquire(spi->bus->mutex, osWaitForever);
    assert(ret_os == osOK);

    /* If not the same config as current, re-configure SPI bus */
    if (memcmp(&spi->bus->config_owner,
                &spi->config, sizeof(elab_spi_config_t)) != 0)
    {
        ret = spi->bus->ops->config(spi, &spi->config);
        if (ret != ELAB_OK)
        {
            goto exit;
        }

        /* Set SPI bus owner */
        spi->bus->config_owner = spi->config;
    }

    /* Initial message. */
    msg.buff_send = buff_send;
    msg.buff_recv = buff_recv;
    msg.size = size;

    /* Disable the SPI device. */
    spi->ops->enable(spi, true);
    /* Transfer message. */
    ret = spi->bus->ops->xfer(spi, &msg);
    /* Disable the SPI device. */
    spi->ops->enable(spi, false);

exit:
    ret_os = osMutexRelease(spi->bus->mutex);
    assert(ret_os == osOK);
    (void)ret_os;

    return ret;
}

/**
  * @brief  Receive data form SPI device.
  * @param  me      The EDF device handle.
  * @param  buff    The SPI receiving buffer.
  * @param  size    receiving size.
  * @retval See elab_err_t
  */
elab_err_t elab_spi_recv(elab_device_t *me, void *buff, uint32_t size)
{
    assert(me != NULL);
    assert(buff != NULL);
    assert(size != 0);

    return elab_spi_xfer(me, NULL, buff, size);
}

/**
  * @brief  Send data form SPI device
  * @param  me      The EDF device handle.
  * @param  buffer  The SPI sending buffer.
  * @param  size    The sending buffer size.
  * @retval See elab_err_t
  */
elab_err_t elab_spi_send(elab_device_t *me, const void *buffer, uint32_t size)
{
    assert(me != NULL);
    assert(buffer != NULL);
    assert(size != 0);

    return elab_spi_xfer(me, buffer, NULL, size);
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
