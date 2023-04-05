/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_spi.h"
#include "elab_assert.h"

/* Private function prototypes -----------------------------------------------*/
static int32_t _bus_read(elab_device_t *me,
                            uint32_t pos, void *buffer, uint32_t size);
static int32_t _bus_write(elab_device_t *me,
                            uint32_t pos, const void *buffer, uint32_t size);
static int32_t _read(elab_device_t *me,
                            uint32_t pos, void *buffer, uint32_t size);
static int32_t _write(elab_device_t *me,
                            uint32_t pos, const void *buffer, uint32_t size);

/* Private variables ---------------------------------------------------------*/
static const elab_dev_ops_t spi_bus_ops =
{
    .enable = NULL,
    .read = _bus_read,
    .write = _bus_write,
#if (ELAB_DEV_PALTFORM == 1)
    .poll = NULL,
#endif
};

static const elab_dev_ops_t spi_device_ops =
{
    .enable = NULL,
    .read = _read,
    .write = _write,
#if (ELAB_DEV_PALTFORM == 1)
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
                            const char *name, const elab_spi_ops_t *ops)
{
    elab_assert(bus != NULL);
    elab_assert(name != NULL);
    elab_assert(ops != NULL);

    elab_device_t *device = &bus->super;

    /* initialize device interface. */
    device->ops = &spi_bus_ops;
    bus->spi_ops = ops;

    /* Initialize mutex mutex */
    const osMutexAttr_t mutex_attr =
    {
        "spi_bus_mutex",
        osMutexPrioInherit,
        NULL,
        0U
    };
    bus->mutex = osMutexNew(&mutex_attr);
    elab_assert(NULL != bus->mutex);

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
  * @brief  SPI device init and register to platform device
  * @param  dev  The pointer of platform dev
  * @param  name Device name
  * @retval See elab_err_t
  */
void elab_spi_register(elab_spi_t *device, const char *name,
                            const char *bus_name, void *user_data)
{
    elab_assert(device != NULL);
    elab_assert(name != NULL);
    elab_assert(bus_name != NULL);
    elab_assert(user_data != NULL);
 
    elab_spi_bus_t *bus = (elab_spi_bus_t *)elab_device_find(bus_name);
    elab_assert(bus != NULL);

    device->bus = bus;

    /* Register to device manager. */
    elab_device_attr_t attr_spi =
    {
        .name = name,
        .sole = true,
        .type = ELAB_DEVICE_SPI,
    };
    elab_device_register(&device->super, &attr_spi);
}

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  platform SPI bus dev read function
  * @param  pdev The pointer of platform dev
  * @param  pos  Position
  * @param  pbuf The pointer of buffer
  * @param  size Expected read length
  * @retval Auctual read length
  */
static int32_t _bus_read(elab_device_t *me,
                            uint32_t pos, void *buffer, uint32_t size)
{
    elab_spi_bus_t *bus;

    bus = (elab_spi_bus_t *)me;
    elab_assert(bus != NULL);
    elab_assert(bus->owner != NULL);

    return elab_spi_xfer(bus->owner, NULL, buffer, size);
}

/**
  * @brief  SPI bus dev write function
  * @param  pdev The pointer of platform dev
  * @param  pos  Position
  * @param  pbuf The pointer of buffer
  * @param  size Expected write length
  * @retval Auctual write length
  */
static int32_t _bus_write(elab_device_t *me,
                            uint32_t pos, const void *buffer, uint32_t size)
{
    elab_spi_bus_t *bus;

    bus = (elab_spi_bus_t *)me;
    elab_assert(bus != NULL);
    elab_assert(bus->owner != NULL);

    return elab_spi_xfer(bus->owner, buffer, NULL, size);
}

/**
  * @brief  SPI device reading function.
  * @param  me      The pointer of device.
  * @param  pos     Position
  * @param  buffer  The pointer of buffer
  * @param  size    Expected read length
  * @retval Auctual read length
  */
static int32_t _read(elab_device_t *me,
                            uint32_t pos, void *buffer, uint32_t size)
{
    elab_spi_t *device;

    device = (elab_spi_t *)me;
    elab_assert(device != NULL);
    elab_assert(device->bus != NULL);

    return elab_spi_xfer(device, NULL, buffer, size);
}

/**
  * @brief  SPI dev write function
  * @param  pdev The pointer of device.
  * @param  pos  Position
  * @param  pbuf The pointer of buffer
  * @param  size Expected write length
  * @retval Auctual write length
  */
static int32_t _write(elab_device_t *me,
                        uint32_t pos, const void *buffer, uint32_t size)
{
    elab_spi_t *device = (elab_spi_t *)me;
    elab_assert(device != NULL);
    elab_assert(device->bus != NULL);

    return elab_spi_xfer(device, buffer, NULL, size);
}

/* ----------------------------- end of file -------------------------------- */
