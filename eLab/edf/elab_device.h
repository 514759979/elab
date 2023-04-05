/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ELAB_DEVICE_H
#define ELAB_DEVICE_H

/* includes ----------------------------------------------------------------- */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "elab_def.h"
#include "cmsis_os.h"

/* public config ------------------------------------------------------------ */
#define ELAB_DEV_NUM_MAX                (32)
#define ELAB_DEV_PALTFORM               (0)     /* 0 - RTOS, 1 - Polling */

/* public define ------------------------------------------------------------ */
enum elab_device_type
{
    ELAB_DEVICE_NULL = 0,

    ELAB_DEVICE_PIN,
    ELAB_DEVICE_PWM,
    ELAB_DEVICE_ADC,
    ELAB_DEVICE_DAC,
    ELAB_DEVICE_UART,
    ELAB_DEVICE_RS485,
    ELAB_DEVICE_I2C_BUS,
    ELAB_DEVICE_I2C,
    ELAB_DEVICE_SPI_BUS,
    ELAB_DEVICE_SPI,
    ELAB_DEVICE_CAN,
    ELAB_DEVICE_WATCHDOG,
    ELAB_DEVICE_RTC,
    ELAB_DEVICE_UNKNOWN,

    ELAB_DEVICE_NORMAL_MAX,
};

enum elab_device_layer
{
    ELAB_DEV_LAYER_NORAML = 0,                /* On-chip device mode */
    ELAB_DEV_LAYER_USER,                      /* User defined device mode */
};

enum elab_device_boot_level
{
    ELAB_DEV_BOOT_L0 = 0,                       /* The lowest */
    ELAB_DEV_BOOT_L1,
    ELAB_DEV_BOOT_L2,
    ELAB_DEV_BOOT_L3,
    ELAB_DEV_BOOT_L4,
    ELAB_DEV_BOOT_L5,
    ELAB_DEV_BOOT_L6,
    ELAB_DEV_BOOT_L7,
};

/* public typedef ----------------------------------------------------------- */
typedef struct elab_device_attr
{
    const char *name;
    bool sole;
    uint8_t type;
} elab_device_attr_t;

typedef struct elab_driver
{
    struct elab_driver *next;
    const char *name;
    void *ops;
    uint8_t type;
    uint8_t layer;
    uint8_t level;
    bool enabled;
} elab_driver_t;

typedef struct elab_device
{
    elab_device_attr_t attr;

    uint8_t enable_count;
#if (ELAB_DEV_PALTFORM == 0)
    osMutexId_t mutex;
#endif

    /* common device interface */
    const struct elab_dev_ops *ops;
    elab_driver_t *drivers;
    void *user_data;
} elab_device_t;

typedef struct elab_dev_ops
{
    elab_err_t (* enable)(elab_device_t *me, bool status);
    int32_t (* read)(elab_device_t *me, uint32_t pos, void *buffer, uint32_t size);
    int32_t (* write)(elab_device_t *me, uint32_t pos, const void *buffer, uint32_t size);
#if (ELAB_DEV_PALTFORM == 1)
    void (* poll)(elab_device_t *me);
#endif
} elab_dev_ops_t;

/* public functions --------------------------------------------------------- */
void elab_device_register(elab_device_t *me, elab_device_attr_t *attr);
void elab_device_boot(uint8_t mode);

elab_device_t *elab_device_find(const char *name);
/* TODO */
void elab_device_add_driver(const char *name, elab_driver_t *driver);
/* TODO */
void elab_device_driver_enable(const char *name, const char *driver);
elab_err_t elab_device_enable(elab_device_t *me, bool status);
void elab_device_mutex_lock(elab_device_t *me, bool status);
int32_t elab_device_read(elab_device_t *me, uint32_t pos, void *buffer, uint32_t size);
int32_t elab_device_write(elab_device_t *me, uint32_t pos, const void *buffer, uint32_t size);

#define elab_devide_open(_dev)              elab_device_enable(_dev, true)
#define elab_devide_close(_dev)             elab_device_enable(_dev, false)

#define elab_device_lock(_dev)              elab_device_mutex_lock(_dev, true)
#define elab_device_unlock(_dev)            elab_device_mutex_unlock(_dev, false)

#endif /* ELAB_DEVICE_H */

/* ----------------------------- end of file -------------------------------- */

