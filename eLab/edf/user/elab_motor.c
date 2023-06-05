/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include <math.h>
#include <stdlib.h>
#include "elab_motor.h"
#include "elab_device.h"
#include "elab_assert.h"

ELAB_TAG("EdfMotor");

/* private typedef ---------------------------------------------------------- */
enum edev_motor_state
{
    EDEV_MOTOR_INIT = 0,
    EDEV_MOTOR_RUN,
    EDEV_MOTOR_EMG_STOP,

    EDEV_MOTOR_MAX
};

/* Private variables ---------------------------------------------------------*/
static const elab_dev_ops_t _motor_ops =
{
    .enable = NULL,
    .read = NULL,
    .write = NULL,
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
    .poll = NULL,
#endif
};

/* public function ---------------------------------------------------------- */
void elab_motor_config_ratio(const char *const name, uint32_t ratio)
{
    elab_device_t *me = elab_device_find(name);
    elab_assert(me != NULL);

    elab_device_lock(me);
    elab_motor_t *motor = (elab_motor_t *)me;
    motor->ratio = ratio;
    elab_device_unlock(me);
}

void elab_motor_config_diameter(const char *const name, float diameter)
{
    elab_device_t *me = elab_device_find(name);
    elab_assert(me != NULL);

    elab_device_lock(me);
    elab_motor_t *motor = (elab_motor_t *)me;
    motor->diameter = diameter;
    elab_device_unlock(me);
}

elab_err_t elab_motor_enable(elab_device_t *const me, bool status)
{
    elab_err_t ret = ELAB_OK;

    elab_device_lock(me);
    elab_motor_t *motor = (elab_motor_t *)me;
    if (motor->ops->ready(motor))
    {
        if ((motor->state == EDEV_MOTOR_INIT ||
                motor->state == EDEV_MOTOR_EMG_STOP) &&
                status)
        {
            ret = motor->ops->enable(motor, status);
            if (ret == ELAB_OK)
            {
                motor->state = EDEV_MOTOR_RUN;
            }
        }
        else if (motor->state == EDEV_MOTOR_RUN && !status)
        {
            ret = motor->ops->enable(motor, status);
            if (ret == ELAB_OK)
            {
                motor->state = EDEV_MOTOR_INIT;
            }
        }
    }
    elab_device_unlock(me);

    return ret;
}

elab_err_t elab_motor_emg_stop(elab_device_t *const me)
{
    elab_err_t ret = ELAB_OK;

    elab_device_lock(me);
    elab_motor_t *motor = (elab_motor_t *)me;
    if (motor->state == EDEV_MOTOR_RUN)
    {
        ret = motor->ops->emg_stop(motor);
        if (ret == ELAB_OK)
        {
            motor->state = EDEV_MOTOR_EMG_STOP;
        }
    }
    elab_device_unlock(me);

    return ret;
}

elab_err_t elab_motor_get_error(elab_device_t *const me, uint32_t *error_out)
{
    elab_err_t ret = ELAB_OK;

    elab_device_lock(me);
    elab_motor_t *motor = (elab_motor_t *)me;
    ret = motor->ops->get_error(motor, error_out);
    elab_device_unlock(me);

    return ret;
}

elab_err_t elab_motor_set_speed(elab_device_t *const me, float speed)
{
    elab_err_t ret = ELAB_OK;

    elab_device_lock(me);

    elab_motor_t *motor = (elab_motor_t *)me;
    if (motor->ops->ready(motor))
    {
        int32_t cmd_speed =
            (int32_t)(speed / motor->diameter / M_PI * (float)motor->ratio);
        if (cmd_speed != motor->speed_cmd)
        {
            ret = motor->ops->set_speed(motor, cmd_speed);
            if (ret == ELAB_OK)
            {
                motor->speed_cmd = cmd_speed;
            }
        }
    }

    elab_device_unlock(me);

    return ret;
}

elab_err_t elab_motor_get_speed(elab_device_t *const me, float *speed)
{
    elab_err_t ret = ELAB_OK;

    elab_device_lock(me);

    elab_motor_t *motor = (elab_motor_t *)me;
    if (motor->ops->ready(motor))
    {
        int32_t act_speed;
        ret = motor->ops->get_speed(motor, &act_speed);
        if (ret == ELAB_OK)
        {
            motor->speed_current = act_speed;
            *speed = ((float)(act_speed / motor->ratio) * M_PI * motor->diameter);
        }
        else
        {
            *speed = 0.0;
        }
    }

    elab_device_unlock(me);

    return ret;
}

void elab_motor_init(elab_motor_t *const me, const char *name,
                        elab_motor_ops_t *ops, void *user_data)
{
    me->ops = ops;
    me->speed_cmd = 0;
    me->speed_current = 0;
    me->ratio = 1;
    me->diameter = 0.2;
    me->state = EDEV_MOTOR_INIT;

    me->super.ops = &_motor_ops;
    me->super.user_data = user_data;

    /* register to device manager */
    elab_device_attr_t attr_motor =
    {
        .name = name,
        .sole = false,
        .type = ELAB_DEVICE_UNKNOWN,
    };
    elab_device_register(&me->super, &attr_motor);
}

/* ----------------------------- end of file -------------------------------- */