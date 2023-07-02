/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ELAB_BUTTON_H
#define ELAB_BUTTON_H

/* include ------------------------------------------------------------------ */
#include "../elab_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/* private typedef ---------------------------------------------------------- */
enum elab_button_event_type
{
    ELAB_BUTTON_EVT_CLICK = 0,
    ELAB_BUTTON_EVT_DOUBLE_CLICK,
    ELAB_BUTTON_EVT_LONGPRESS,
};

#if (ELAB_QPC_EN != 0)
typedef struct elab_button_event
{
    QEvt super;
    uint8_t type;
} elab_button_event_t;
#endif

typedef struct elab_button
{
    elab_device_t super;
    elab_button_event_cb_t cb;
    uint32_t event_id;
    
    struct elab_button_ops *ops;
} elab_button_t;

typedef void (* elab_button_event_cb_t)(uint8_t event_id);

typedef struct elab_button_ops
{
    bool (* read_status)(elab_button_t *const me, bool status);
} elab_button_ops_t;

#define ELAB_BUTTON_CAST(_dev)        ((elab_button_t *)_dev)

/* public function ---------------------------------------------------------- */
/* Button class functions */
void elab_button_set_cb(elab_device_t *const me, elab_button_event_cb_t cb);
#if (ELAB_QPC_EN != 0)
void elab_button_set_event(elab_device_t *const me, uint32_t event_id);
uint8_t elab_button_get_type(QEvt const * const e);
#endif

/* Button exporting. */
void elab_button_init(elab_button_t *const me, const char *name,
                        elab_button_ops_t *ops, void *user_data);

#ifdef __cplusplus
}
#endif

#endif  /* ELAB_BUTTON_H */

/* ----------------------------- end of file -------------------------------- */
