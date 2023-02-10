/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "event_def.h"
#include "qpc.h"
#include "cmsis_os.h"
#include "elab.h"

/* private variables ---------------------------------------------------------*/
typedef struct ao_led
{
    QActive super;
    QTimeEvt timeEvt;
    bool status;
} ao_led_t;

/* private state function prototypes -----------------------------------------*/
static QState ao_led_initial(ao_led_t * const me, void const * const par);
static QState ao_led_on(ao_led_t * const me, QEvt const * const e);
static QState ao_led_off(ao_led_t * const me, QEvt const * const e);

/* private variables ---------------------------------------------------------*/
static ao_led_t led;

HSM_EXPORT(sm_led, &led, ao_led_initial, osPriorityNormal, 32, 1024);

/* private state function ----------------------------------------------------*/
static QState ao_led_initial(ao_led_t * const me, void const * const par)
{
    (void)par;
    
    QTimeEvt_ctorX(&me->timeEvt, &me->super, Q_QPC_TEST_SIG, 0U);
    QTimeEvt_armX(&me->timeEvt, 1U, 500);

    return Q_TRAN(&ao_led_off);
}

static QState ao_led_on(ao_led_t * const me, QEvt const * const e)
{
    QState status_;

    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            me->status = true;
            printf("Led on!\n");
            status_ = Q_HANDLED();
            break;
        }

        case Q_QPC_TEST_SIG:
        {
            status_ = Q_TRAN(&ao_led_off);
            break;
        }

        default:
        {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }

    return status_;
}

static QState ao_led_off(ao_led_t * const me, QEvt const * const e)
{
    QState status_;

    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            me->status = false;
            printf("Led off!\n");
            status_ = Q_HANDLED();
            break;
        }

        case Q_QPC_TEST_SIG:
        {
            status_ = Q_TRAN(&ao_led_on);
            break;
        }

        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }

    return status_;
}

/* ----------------------------- end of file -------------------------------- */
