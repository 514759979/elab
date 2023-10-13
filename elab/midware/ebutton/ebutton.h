/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ELAB_BUTTON_H
#define ELAB_BUTTON_H

/* include ------------------------------------------------------------------ */
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* private typedef ---------------------------------------------------------- */
enum elab_button_event_type
{
    ELAB_BUTTON_EVT_PRESS_DOWN = 0,
    ELAB_BUTTON_EVT_PRESS_UP,
    ELAB_BUTTON_EVT_REPEAT,
    ELAB_BUTTON_EVT_CLICK = 0,
    ELAB_BUTTON_EVT_DOUBLE_CLICK,
    ELAB_BUTTON_EVT_LONGPRESS_START,
    ELAB_BUTTON_EVT_LONGPRESS_HOLD,

    ELAB_BUTTON_EVT_MAX,
    ELAB_BUTTON_EVT_NONE,
};

typedef void (* elab_button_cb_t)(void);
typedef bool (* elab_button_status_t)(void);

typedef struct elab_button_action
{
    const char *name_button0;
    const char *name_button1;
    const char *name_button2;
    uint32_t time;
    uint8_t event_type;
    uint8_t state;
    uint8_t repeat;
    uint32_t event_id;
    elab_button_cb_t cb;
    struct elab_button_action *next;
    uint32_t time_count;
} elab_button_action_t;

typedef struct elab_button
{
    bool status;
    uint8_t flag;
    const char *name;
    elab_button_status_t ops;
    struct elab_button *next;
} elab_button_t;

/* public function ---------------------------------------------------------- */
/* Button class functions */
void elab_button_attach(elab_button_action_t * const action);

/* Button exporting. */
void elab_button_init(elab_button_t *const me,
                        const char *name, elab_button_status_t ops);

#ifdef __cplusplus
}
#endif

#endif  /* ELAB_BUTTON_H */

/* ----------------------------- end of file -------------------------------- */
