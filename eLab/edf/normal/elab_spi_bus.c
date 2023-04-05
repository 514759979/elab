/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_spi.h"
#include "elab_assert.h"

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Attach a device on SPI bus
  * @param  device    The pointer of EDF SPI dev
  * @param  name      Device name
  * @param  bus_name  SPI bus name
  * @param  user_data The pointer of user data
  * @retval See elab_err_t
  */
elab_err_t elab_spi_bus_attach_device(elab_spi_t *device, const char *name,
                                        const char *bus_name, void *user_data)
{
    elab_err_t ret = ELAB_OK;
    elab_device_t *bus;

    /* get physical spi bus */
    bus = elab_device_find(bus_name);
    elab_assert(NULL != bus);

    device->bus = (elab_spi_bus_t *)bus;

    memset(&device->config, 0, sizeof(device->config));
    device->super.user_data = user_data;

    ret = ELAB_OK;

exit:
    return ret;
}

/**
  * @brief  Set configuration on SPI device
  * @param  device The pointer of EDF SPI dev
  * @param  cfg    The poingter of cfg
  * @retval See elab_err_t
  */
elab_err_t elab_spi_config(elab_spi_t *device, elab_spi_config_t *cfg)
{
    elab_err_t ret = ELAB_OK;
    osStatus_t result = osOK;

    elab_assert(device != NULL);

    /* set configuration */
    device->config.data_width = cfg->data_width;
    device->config.mode       = cfg->mode & ELAB_SPI_MODE_MASK ;
    device->config.max_hz     = cfg->max_hz ;

    if (NULL == device->bus)
    {
        ret = ELAB_ERROR;
        goto exit;
    }

    result = osMutexAcquire(device->bus->mutex, osWaitForever);
    if (result != osOK)
    {
        ret = ELAB_ERR_NO_SYSTEM;
        goto exit;
    }

    if (device->bus->owner == device)
    {
        ret = device->bus->spi_ops->config(device, &device->config);
    }

    /* release mutex */
    osMutexRelease(device->bus->mutex);

exit:
    return ret;
}

/**
  * @brief  Send data then send data from SPI device.
  * @param  device       The pointer of EDF SPI dev
  * @param  send_buf1    The poingter send buf1
  * @param  send_length1 Send length1
  * @param  send_buf2    The poingter send buf2
  * @param  send_length2 Send length2
  * @retval See elab_err_t
  */
elab_err_t elab_spi_send_then_send
                (elab_spi_t *device, const void *send_buf1, uint32_t send_length1,
                 const void *send_buf2, uint32_t send_length2)
{
    elab_err_t ret = ELAB_OK;
    elab_spi_msg_t message;

    elab_assert(device != NULL);
    elab_assert(device->bus != NULL);

    osStatus_t result = osMutexAcquire(device->bus->mutex, osWaitForever);
    if (result != osOK)
    {
        ret = ELAB_ERR_NO_SYSTEM;
        goto exit;
    }

    if (device->bus->owner != device)
    {
        /* not the same owner as current, re-configure SPI bus */
        ret = device->bus->spi_ops->config(device, &device->config);

        if (ret != ELAB_OK)
        {
            goto exit2;
        }

        /* set SPI bus owner */
        device->bus->owner = device;
    }

    /* send data1 */
    message.send_buf   = send_buf1;
    message.recv_buf   = NULL;
    message.length     = send_length1;
    message.cs_take    = 1;
    message.cs_release = 0;
    message.next       = NULL;

    ret = device->bus->spi_ops->xfer(device, &message);
    if (ret != ELAB_OK) goto exit2;

    /* send data2 */
    message.send_buf   = send_buf2;
    message.recv_buf   = NULL;
    message.length     = send_length2;
    message.cs_take    = 0;
    message.cs_release = 1;
    message.next       = NULL;

    ret = device->bus->spi_ops->xfer(device, &message);

exit2:
    osMutexRelease(device->bus->mutex);
exit:
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
elab_err_t elab_spi_send_then_recv
                (elab_spi_t *device, const void *send_buf, uint32_t send_length,
                 void *recv_buf, uint32_t recv_length)
{
    elab_err_t ret = ELAB_OK;
    elab_spi_msg_t message;

    elab_assert(device != NULL);
    elab_assert(device->bus != NULL);

    osStatus_t result = osMutexAcquire(device->bus->mutex, osWaitForever);
    if (result != osOK)
    {
        ret = ELAB_ERR_NO_SYSTEM;
        goto exit;
    }

    if (device->bus->owner != device)
    {
        /* not the same owner as current, re-configure SPI bus */
        ret = device->bus->spi_ops->config(device, &device->config);
        if (ret != ELAB_OK) goto exit2;
        /* set SPI bus owner */
        device->bus->owner = device;
    }

    /* send data */
    message.send_buf   = send_buf;
    message.recv_buf   = NULL;
    message.length     = send_length;
    message.cs_take    = 1;
    message.cs_release = 0;
    message.next       = NULL;

    ret = device->bus->spi_ops->xfer(device, &message);
    if (ret != ELAB_OK) goto exit2;

    /* recv data */
    message.send_buf   = NULL;
    message.recv_buf   = recv_buf;
    message.length     = recv_length;
    message.cs_take    = 0;
    message.cs_release = 1;
    message.next       = NULL;

    ret = device->bus->spi_ops->xfer(device, &message);

exit2:
    osMutexRelease(device->bus->mutex);
exit:
    return ret;
}

/**
 * This function transmits data to SPI device.
 *
 * @param device the SPI device attached to SPI bus
 * @param send_buf the buffer to be transmitted to SPI device.
 * @param recv_buf the buffer to save received data from SPI device.
 * @param length the length of transmitted data.
 *
 * @return See elab_err_t
 */
elab_err_t elab_spi_xfer(elab_spi_t *device,
                            const void *send_buf, void *recv_buf, uint32_t length)
{
    elab_err_t ret = ELAB_OK;
    elab_spi_msg_t message;

    elab_assert(device != NULL);
    elab_assert(device->bus != NULL);

    osStatus_t result = osMutexAcquire(device->bus->mutex, osWaitForever);
    if (result != osOK)
    {
        ret = ELAB_ERR_NO_SYSTEM;
        goto exit;
    }

    if (device->bus->owner != device)
    {
        /* not the same owner as current, re-configure SPI bus */
        ret = device->bus->spi_ops->config(device, &device->config);
        if (ret != ELAB_OK) goto exit2;

        /* set SPI bus owner */
        device->bus->owner = device;
    }

    /* initial message */
    message.send_buf   = send_buf;
    message.recv_buf   = recv_buf;
    message.length     = length;
    message.cs_take    = 1;
    message.cs_release = 1;
    message.next       = NULL;

    /* transfer message */
    ret = device->bus->spi_ops->xfer(device, &message);

exit2:
    osMutexRelease(device->bus->mutex);
exit:
    return ret;
}

/**
 * This function transfers a message list to the SPI device.
 *
 * @param device the SPI device attached to SPI bus
 * @param message the message list to be transmitted to SPI device
 *
 * @return See elab_err_t
 */
elab_err_t elab_spi_transfer_message
(elab_spi_t  *device, elab_spi_msg_t *message)
{
    elab_err_t ret = ELAB_OK;
    elab_spi_msg_t *index;

    elab_assert(device != NULL);

    /* get first message */
    index = message;
    if (NULL == index)
    {
        ret = ELAB_ERR_INVALID;
        goto exit;
    }

    osStatus_t result = osMutexAcquire(device->bus->mutex, osWaitForever);
    if (result != osOK)
    {
        ret = ELAB_ERR_NO_SYSTEM;
        goto exit;
    }

    /* configure SPI bus */
    if (device->bus->owner != device)
    {
        /* not the same owner as current, re-configure SPI bus */
        ret = device->bus->spi_ops->config(device, &device->config);

        if (ret != ELAB_OK) goto exit2;

        /* set SPI bus owner */
        device->bus->owner = device;
    }

    /* transmit each SPI message */
    while (index != NULL)
    {
        /* transmit SPI message */
        ret = device->bus->spi_ops->xfer(device, index);
        if (ret != ELAB_OK) break;

        index = index->next;
    }

exit2:
    /* release bus mutex */
    osMutexRelease(device->bus->mutex);
exit:
    return ret;
}

/**
 * This function takes SPI bus.
 *
 * @param device the SPI device attached to SPI bus
 *
 * @return ELAB_OK on taken SPI bus successfully. others on taken SPI bus
 * failed.
 */
elab_err_t elab_spi_take_bus(elab_spi_t *device)
{
    elab_err_t ret = ELAB_OK;

    elab_assert(device != NULL);
    elab_assert(device->bus != NULL);

    osStatus_t result = osMutexAcquire(device->bus->mutex, osWaitForever);
    if (result != osOK)
    {
        ret = ELAB_ERR_NO_SYSTEM;
        goto exit;
    }

    /* configure SPI bus */
    if (device->bus->owner != device)
    {
        /* not the same owner as current, re-configure SPI bus */
        ret = device->bus->spi_ops->config(device, &device->config);
        if (ELAB_OK == ret)
        {
            /* set SPI bus owner */
            device->bus->owner = device;
        }
        else
        {
            /* configure SPI bus failed */
            /* release bus mutex */
            osMutexRelease(device->bus->mutex);
        }
    }
exit:
    return ret;
}

/**
 * This function releases SPI bus.
 *
 * @param device the SPI device attached to SPI bus
 *
 * @return ELAB_OK on release SPI bus successfully.
 */
elab_err_t elab_spi_release_bus(elab_spi_t *device)
{
    elab_assert(device != NULL);
    elab_assert(device->bus != NULL);
    elab_assert(device->bus->owner == device);

    /* release bus mutex */
    osMutexRelease(device->bus->mutex);

    return ELAB_OK;
}

/**
 * This function take SPI device (takes CS of SPI device).
 *
 * @param device the SPI device attached to SPI bus
 *
 * @return ELAB_OK on release SPI bus successfully. others on taken SPI bus
 * failed.
 */
elab_err_t elab_spi_take(elab_spi_t *device)
{
    elab_err_t ret;
    elab_spi_msg_t message;

    elab_assert(device != NULL);
    elab_assert(device->bus != NULL);

    memset(&message, 0, sizeof(message));
    message.cs_take = 1;

    ret = device->bus->spi_ops->xfer(device, &message);

    return ret;
}

/**
 * This function releases SPI device (releases CS of SPI device).
 *
 * @param device the SPI device attached to SPI bus
 *
 * @return ELAB_OK on release SPI device successfully.
 */
elab_err_t elab_spi_release(elab_spi_t *device)
{
    elab_err_t ret;
    elab_spi_msg_t message;

    elab_assert(device != NULL);
    elab_assert(device->bus != NULL);

    memset(&message, 0, sizeof(message));
    message.cs_release = 1;

    ret = device->bus->spi_ops->xfer(device, &message);

    return ret;
}

/**
  * @brief  Receive data form SPI device
  * @param  device The pointer of EDF SPI dev
  * @param  recv_buf   The poingter of Receive buffer
  * @param  length     Receive length
  * @retval See elab_err_t
  */
elab_err_t elab_spi_recv(elab_spi_t *device,
                                 void *recv_buf,
                                 uint32_t length)
{
    return elab_spi_xfer(device, NULL, recv_buf, length);
}

/**
  * @brief  Send data form SPI device
  * @param  device The pointer of EDF SPI dev
  * @param  send_buf   The poingter of send buffer
  * @param  length     sent length
  * @retval See elab_err_t
  */
elab_err_t elab_spi_send(elab_spi_t *device,
                                 const void *send_buf,
                                 uint32_t length)
{
    return elab_spi_xfer(device, send_buf, NULL, length);
}

/**
  * @brief  Send 1 byte data and receive 1 byte data form SPI device
  * @param  device The pointer of EDF SPI dev
  * @param  data   Send data
  * @retval Received data
  */
uint8_t elab_spi_sendrecv8(elab_spi_t *device, uint8_t data)
{
    uint8_t value;

    elab_spi_send_then_recv(device, &data, 1, &value, 1);

    return value;
}

/**
  * @brief  Send 2 byte data and receive 2 byte data form SPI device
  * @param  device The pointer of EDF SPI dev
  * @param  data   Send data
  * @retval Received data
  */
uint16_t elab_spi_sendrecv16(elab_spi_t *device, uint16_t data)
{
    uint16_t value;

    elab_spi_send_then_recv(device, &data, 2, &value, 2);

    return value;
}

/**
 * This function appends a message to the SPI message list.
 *
 * @param list the SPI message list header.
 * @param msg the message pointer to be appended to the message list.
 */
void elab_spi_message_append(elab_spi_msg_t *list, elab_spi_msg_t *msg)
{
    elab_assert(list != NULL);

    if (msg == NULL)
        return; /* not append */

    while (list->next != NULL)
    {
        list = list->next;
    }

    list->next = msg;
    msg->next = NULL;
}

/* ----------------------------- end of file -------------------------------- */
