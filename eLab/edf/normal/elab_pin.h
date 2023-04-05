/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef EIO_PIN_H
#define EIO_PIN_H

/* includes ----------------------------------------------------------------- */
#include "elab_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public define ------------------------------------------------------------ */
enum pin_mode
{
    PIN_MODE_INPUT = 0,
    PIN_MODE_INPUT_PULLUP,
    PIN_MODE_INPUT_PULLDOWN,
    PIN_MODE_OUTPUT_PP,
    PIN_MODE_OUTPUT_OD,

    PIN_MODE_MAX
};

/* public typedef ----------------------------------------------------------- */
typedef struct eio_pin
{
    elab_device_t super;

    const struct eio_pin_ops *ops;
    uint8_t mode;
    bool status;
} elab_pin_t;

typedef struct eio_pin_ops
{
    void (* init)(elab_pin_t * const me);
    void (* set_mode)(elab_pin_t * const me, uint8_t mode);
    bool (* get_status)(elab_pin_t * const me);
    void (* set_status)(elab_pin_t * const me, bool status);
} eio_pin_ops_t;

/* public functions --------------------------------------------------------- */
/* For low-level driver. */
void eio_pin_register(elab_pin_t * const me,
                        const char *name,
                        const eio_pin_ops_t *ops,
                        void *user_data);

/* For high-level code. */
void eio_pin_set_mode(elab_device_t * const me, uint8_t mode);
bool eio_pin_get_status(elab_device_t * const me);
void eio_pin_set_status(elab_device_t * const me, bool status);

#ifdef __cplusplus
}
#endif

#endif

/* ----------------------------- end of file -------------------------------- */
