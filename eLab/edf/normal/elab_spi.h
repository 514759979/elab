/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef __ELAB_SPI_H
#define __ELAB_SPI_H

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include "elab_device.h"
#include "cmsis_os.h"

/* public macros -------------------------------------------------------------*/
/**
 * At CPOL=0 the base value of the clock is zero
 *  - For CPHA=0, data are captured on the clock's rising edge
 * (low->high transition)
 *    and data are propagated on a falling edge (high->low clock transition).
 *  - For CPHA=1, data are captured on the clock's falling edge and data are
 *    propagated on a rising edge.
 * At CPOL=1 the base value of the clock is one (inversion of CPOL=0)
 *  - For CPHA=0, data are captured on clock's falling edge and data are
 * propagated on a rising edge.
 *  - For CPHA=1, data are captured on clock's rising edge and data are
 * propagated on a falling edge.
 */
#define ELAB_SPI_CPHA     (1 << 0)  /* bit[0]:CPHA, clock phase */
#define ELAB_SPI_CPOL     (1 << 1)  /* bit[1]:CPOL, clock polarity */

#define ELAB_SPI_LSB      (0 << 2)  /* bit[2]: 0-LSB */
#define ELAB_SPI_MSB      (1 << 2)  /* bit[2]: 1-MSB */

#define ELAB_SPI_MASTER   (0 << 3)  /* SPI master device */
#define ELAB_SPI_SLAVE    (1 << 3)  /* SPI slave device */

#define ELAB_SPI_CS_HIGH  (1 << 4)  /* Chip select active high */
#define ELAB_SPI_NO_CS    (1 << 5)  /* No chipselect */
#define ELAB_SPI_3WIRE    (1 << 6)  /* SI/SO pin shared */
#define ELAB_SPI_READY    (1 << 7)  /* Slave pulls low to pause */

#define ELAB_SPI_MODE_MASK                                                     \
        (ELAB_SPI_CPHA | ELAB_SPI_CPOL | ELAB_SPI_MSB | ELAB_SPI_SLAVE |       \
        ELAB_SPI_CS_HIGH | ELAB_SPI_NO_CS | ELAB_SPI_3WIRE |                   \
        ELAB_SPI_READY)

enum elab_spi_mode
{
    ELAB_SPI_MODE_0 = (0 | 0),                          /* CPOL = 0, CPHA = 0 */
    ELAB_SPI_MODE_1 = (0 | ELAB_SPI_CPHA),              /* CPOL = 0, CPHA = 1 */
    ELAB_SPI_MODE_2 = (ELAB_SPI_CPOL | 0),              /* CPOL = 1, CPHA = 0 */
    ELAB_SPI_MODE_3 = (ELAB_SPI_CPOL | ELAB_SPI_CPHA),  /* CPOL = 1, CPHA = 1 */
};

#define ELAB_SPI_BUS_MODE_SPI         (1 << 0)

/* Public types ------------------------------------------------------------- */
/**
 * SPI message structure
 */
typedef struct elab_spi_msg
{
    const void *send_buf;
    void *recv_buf;
    uint32_t length;
    struct elab_spi_msg *next;

    uint32_t cs_take                    : 1;
    uint32_t cs_release                 : 1;
} elab_spi_msg_t;

/**
 * SPI configuration structure
 */
typedef struct elab_spi_config
{
    uint8_t mode;
    uint8_t data_width;
    uint16_t reserved;
    uint32_t max_hz;
} elab_spi_config_t;

/**
 * SPI Virtual BUS, one device must connected to a virtual BUS
 */
struct elab_spi_ops;
typedef struct elab_spi_bus
{
    elab_device_t super;

    uint8_t mode;
    const struct elab_spi_ops *spi_ops;

    osMutexId_t mutex;
    struct elab_spi_device *owner;
} elab_spi_bus_t;

typedef struct elab_spi_device
{
    elab_device_t super;

    elab_spi_bus_t *bus;
    elab_spi_config_t config;
    void *user_data;
} elab_spi_t;

/**
 * SPI operators
 */
typedef struct elab_spi_ops
{
    elab_err_t (* config)(elab_spi_t *const me, elab_spi_config_t *config);
    elab_err_t (* xfer)(elab_spi_t * const me, elab_spi_msg_t *message);
} elab_spi_ops_t;

/* Public functions --------------------------------------------------------- */
void elab_spi_bus_register(elab_spi_bus_t *bus,
                            const char *name, const elab_spi_ops_t *ops);

void elab_spi_register(elab_spi_t *device, const char *name,
                            const char *bus_name, void *user_data);

elab_err_t elab_spi_take_bus(elab_spi_t *device);
elab_err_t elab_spi_release_bus(elab_spi_t *device);
elab_err_t elab_spi_take(elab_spi_t *device);
elab_err_t elab_spi_release(elab_spi_t *device);

/* Before doing anything with the SPI device, you first need to call this function 
 * to configure the bus. */
elab_err_t elab_spi_config(elab_spi_t *device, elab_spi_config_t *cfg);
elab_err_t elab_spi_send_then_recv(elab_spi_t *device,
                                    const void *send_buf, uint32_t send_length,
                                    void *recv_buf, uint32_t recv_length);
elab_err_t elab_spi_send_then_send(elab_spi_t *device,
                                    const void *buff1, uint32_t size1,
                                    const void *buff2, uint32_t size2);
elab_err_t elab_spi_xfer(elab_spi_t *device,
                            const void *send_buf, void *recv_buf, uint32_t length);
elab_err_t elab_spi_xfer_msg(elab_spi_t *device, elab_spi_msg_t *message);
elab_err_t elab_spi_recv(elab_spi_t *device, void *recv_buf, uint32_t length);
elab_err_t elab_spi_send(elab_spi_t *device, const void *buffer, uint32_t size);
uint8_t elab_spi_send_recv_u8(elab_spi_t *device, uint8_t data);
uint16_t elab_spi_send_recv_u16(elab_spi_t *device, uint16_t data);
void elab_spi_msg_append(elab_spi_msg_t *list, elab_spi_msg_t *message);

#endif /* __ELAB_SPI_H */

/* ----------------------------- end of file -------------------------------- */
