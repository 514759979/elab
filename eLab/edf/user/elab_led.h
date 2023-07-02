/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ELAB_LED_H
#define ELAB_LED_H

/* include ------------------------------------------------------------------ */
#include "../elab_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/* private typedef ---------------------------------------------------------- */
typedef struct elab_led
{
    elab_device_t super;
    osTimerId_t timer;
    uint32_t period_ms;
    uint32_t time_out;
    uint8_t mode;
    uint8_t value;
    uint8_t value_count;
    bool status;

    struct elab_led_ops *ops;
} elab_led_t;

typedef struct elab_led_ops
{
    elab_err_t (* set_status)(elab_led_t *const me, bool status);
} elab_led_ops_t;

#define ELAB_LED_CAST(_dev)           ((elab_led_t *)_dev)

/* public function ---------------------------------------------------------- */
/* LED class functions */
void elab_led_set_status(elab_device_t *const me, bool status);
void elab_led_toggle(elab_device_t *const me, uint32_t period_ms);
void elab_led_set_value(elab_device_t *const me, uint8_t value);

/* LED exporting. */
void elab_led_init(elab_led_t *const me, const char *name,
                        elab_led_ops_t *ops, void *user_data);

#ifdef __cplusplus
}
#endif

#endif  /* ELAB_LED_H */

/* ----------------------------- end of file -------------------------------- */
