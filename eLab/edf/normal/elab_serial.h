/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ELAB_SERIAL_H
#define ELAB_SERIAL_H

/* includes ----------------------------------------------------------------- */
#include "elab_device.h"

/* Exported macros -----------------------------------------------------------*/
enum elab_serial_data_bits
{
    ELAB_SERIAL_DATA_BITS_5 = 5,
    ELAB_SERIAL_DATA_BITS_6,
    ELAB_SERIAL_DATA_BITS_7,
    ELAB_SERIAL_DATA_BITS_8,
    ELAB_SERIAL_DATA_BITS_9,
};

enum elab_serail_stop_bits
{
    ELAB_SERIAL_STOP_BITS_1 = 0,
    ELAB_SERIAL_STOP_BITS_2,
    ELAB_SERIAL_STOP_BITS_3,
    ELAB_SERIAL_STOP_BITS_4,
};

enum elab_serial_parity
{
    ELAB_SERIAL_PARITY_NONE = 0,
    ELAB_SERIAL_PARITY_ODD,
    ELAB_SERIAL_PARITY_EVEN,
};

enum elab_serail_mode
{
    ELAB_SERIAL_MODE_FULL_DUPLEX = 0,
    ELAB_SERIAL_MODE_HALF_DUPLEX,
};

/* Default config for elab_serial_config_t */
#define ELAB_SERIAL_ATTR_DEFAULT                                               \
{                                                                              \
    115200,                                         /* 115200 bits/s */        \
    256,                                            /* rx buffer size */       \
    1024,                                           /* tx buffer size */       \
    ELAB_SERIAL_DATA_BITS_8,                        /* 8 databits */           \
    ELAB_SERIAL_STOP_BITS_1,                        /* 1 stopbit */            \
    ELAB_SERIAL_PARITY_NONE,                        /* No parity  */           \
    ELAB_SERIAL_MODE_FULL_DUPLEX,                   /* Full / half duplex */   \
    0                                                                          \
}

/* Exported types ------------------------------------------------------------*/
typedef struct elab_serial_attr
{
    uint32_t baud_rate;

    uint32_t rx_bufsz                :16;
    uint32_t tx_bufsz                :16;
    uint32_t data_bits               :4;
    uint32_t stop_bits               :2;
    uint32_t parity                  :2;
    uint32_t mode                    :1;
    uint32_t reserved                :5;
} elab_serial_attr_t;

typedef struct elab_serail
{
    elab_device_t super;

    const struct elab_serial_ops *ops;
    elab_serial_attr_t attr;
} elab_serial_t;

typedef struct elab_serial_ops
{
    elab_err_t (* enable)(elab_serial_t *serial, bool status);
    int32_t (* read)(elab_serial_t *serial, void *buffer, uint32_t size);
    int32_t (* write)(elab_serial_t *serial, const void *buffer, uint32_t size);
    void (* set_tx)(elab_serial_t *serial, bool status);
    elab_err_t (* config)(elab_serial_t *serial, elab_serial_attr_t *attr);
} elab_serial_ops_t;

/* Exported functions --------------------------------------------------------*/
void elab_serial_register(elab_serial_t *serial, const char *name,
                            elab_serial_ops_t *ops, void *user_data);

/* For high level program. */
void elab_serial_set_mode(elab_serial_t *serial, uint8_t mode);
void elab_serial_set_baudrate(elab_serial_t *serial, uint32_t baudrate);
void elab_serial_set_attr(elab_serial_t *serial, elab_serial_attr_t *attr);
elab_serial_attr_t elab_serial_get_attr(elab_serial_t *serial);

void elab_serial_xfer(elab_serial_t *serial,
                        void *buff_tx, uint32_t size_tx,
                        void *buff_rx, uint32_t size_rx);

#endif /* ELAB_SERIAL_H */

/* ----------------------------- end of file -------------------------------- */
