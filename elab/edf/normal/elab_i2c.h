/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef __ELAB_I2C_H
#define __ELAB_I2C_H

/* includes ----------------------------------------------------------------- */
#include "../elab_device.h"
#include "../../os/cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
typedef struct elab_i2c_msg
{
    bool write;
    uint16_t len;
    uint8_t *buffer;
} elab_i2c_msg_t;

typedef struct elab_i2c_bus_config
{
    uint32_t clock;
    bool addr_10bit;
} elab_i2c_bus_config_t;

typedef struct elab_i2c_config
{
    uint32_t clock;
    bool addr_10bit;
    uint16_t addr;
} elab_i2c_config_t;

typedef struct elab_i2c_priv_data
{
    elab_i2c_msg_t *msgs;
    uint32_t number;
} elab_i2c_priv_data_t;

typedef struct elab_i2c_bus
{
    elab_device_t super;

    const struct elab_i2c_bus_ops *ops;
    osMutexId_t mutex;
    elab_i2c_bus_config_t config;
} elab_i2c_bus_t;

typedef struct elab_i2c
{
    elab_device_t super;

    elab_i2c_bus_t *bus;
    elab_i2c_config_t config;
} elab_i2c_t;

typedef struct elab_i2c_bus_ops
{
    int32_t (* xfer)(elab_i2c_bus_t *, uint16_t addr, elab_i2c_msg_t msg);
    elab_err_t (* config)(elab_i2c_bus_t *, elab_i2c_bus_config_t *config);
} elab_i2c_bus_ops_t;

/* Exported functions --------------------------------------------------------*/
void elab_i2c_bus_register(elab_i2c_bus_t *bus,
                            const char *name, const elab_i2c_bus_ops_t *ops,
                            void *user_data);
void elab_i2c_register(elab_i2c_t *device, const char *name, const char *bus_name,
                            elab_i2c_config_t config,
                            void *user_data);

int32_t elab_i2c_xfer_msgs(elab_device_t *me, elab_i2c_msg_t msgs[], uint32_t num);
elab_err_t elab_i2c_xfer(elab_device_t *me, elab_i2c_msg_t msg);

#ifdef __cplusplus
}
#endif

#endif

/* ----------------------------- end of file -------------------------------- */
