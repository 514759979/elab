/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef __ELAB_I2C_H
#define __ELAB_I2C_H

/* includes ----------------------------------------------------------------- */
#include "elab_def.h"
#include "elab_device.h"
#include "cmsis_os.h"

/* Exported macros -----------------------------------------------------------*/
#define elab_i2c_WR                         (0)
#define elab_i2c_RD                         (1u << 0)
#define elab_i2c_ADDR_10BIT                 (1u << 2)  /* this is a ten bit chip address */

#define elab_i2c_DEV_CTRL_RW                (0x21)
#define elab_i2c_DEV_CTRL_CLK               (0x22)

/* Exported types ------------------------------------------------------------*/
typedef struct elab_i2c_msg
{
    uint16_t addr;
    uint16_t flags;
    uint16_t len;
    uint8_t *buffer;
} elab_i2c_msg_t;

typedef struct elab_i2c_priv_data
{
    elab_i2c_msg_t *msgs;
    uint32_t number;
} elab_i2c_priv_data_t;

typedef struct elab_i2c_bus
{
    elab_device_t super;

    const struct elab_i2c_ops *ops;
    osMutexId_t mutex;
    void *priv;
} elab_i2c_bus_t;

typedef struct elab_i2c
{
    elab_device_t super;

    elab_i2c_bus_t *bus;
    uint16_t client_addr;
} elab_i2c_t;

typedef struct elab_i2c_ops
{
    int32_t (*master_xfer)(elab_i2c_bus_t *, elab_i2c_msg_t *, uint32_t);
    int32_t (*slave_xfer)(elab_i2c_bus_t *, elab_i2c_msg_t *, uint32_t);
    elab_err_t (*i2c_bus_control)(elab_i2c_bus_t *, uint32_t, uint32_t);
} elab_i2c_ops_t;

/* Exported functions --------------------------------------------------------*/
void elab_i2c_bus_register(elab_i2c_bus_t *bus,
                            const char *name, const elab_i2c_ops_t *ops);
elab_err_t elab_i2c_bus_init(elab_i2c_bus_t *bus, const char *name);
elab_i2c_bus_t *elab_i2c_bus_device_find(const char *bus_name);

int32_t elab_i2c_xfer(elab_i2c_bus_t *bus, elab_i2c_msg_t msg[], uint32_t num);
elab_err_t elab_i2c_control(elab_i2c_bus_t *bus, uint32_t cmd, uint32_t arg);
elab_err_t elab_i2c_master_send(elab_i2c_bus_t *bus, uint16_t addr,
                                uint16_t flags, const uint8_t *buf, uint32_t count);
elab_err_t elab_i2c_master_recv(elab_i2c_bus_t *bus, uint16_t addr,
                                uint16_t flags, uint8_t *buf, uint32_t count);

#endif

/* ----------------------------- end of file -------------------------------- */
