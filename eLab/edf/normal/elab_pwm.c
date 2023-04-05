
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include "elab_pwm.h"
#include "elab_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("EIO_PWM")

/* private variables -------------------------------------------------------- */
static elab_dev_ops_t _ops =
{
    .enable = NULL,
    .read = NULL,
    .write = NULL,
};

/* public functions --------------------------------------------------------- */
/**
  * @brief  eLab pwm register function.
  * @param  me          this pointer
  * @param  name        pwm's name.
  * @param  ops         ops interface.
  * @param  user_data   User private data.
  * @retval None
  */
void elab_pwm_register(elab_pwm_t * const me,
                        const char *name,
                        const elab_pwm_ops_t *ops,
                        void *user_data)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    elab_assert(ops != NULL);

    elab_device_attr_t attr =
    {
        .name = name,
        .sole = true,
        .type = ELAB_DEVICE_PWM,
    };

    elab_device_register(&me->super, &attr);
    me->super.ops = &_ops;

    me->ops = ops;
    me->frequency = 1000;
    me->duty_ratio = 0;
}

/**
  * @brief  eLab pwm's status turning on function.
  * @param  me      this pointer
  * @param  duty    the input pwm status.
  * @retval None.
  */
void elab_pwm_set_duty(elab_device_t * const me, uint8_t duty_ratio)
{
    elab_assert(me != NULL);
    elab_assert(duty_ratio <= 100);
    elab_assert(me->attr.type == ELAB_DEVICE_PWM);
    
    elab_pwm_t *pwm = (elab_pwm_t *)me;
    if (duty_ratio != pwm->duty_ratio)
    {
        pwm->duty_ratio = duty_ratio;
        pwm->ops->set_duty(pwm, pwm->duty_ratio);
    }
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
