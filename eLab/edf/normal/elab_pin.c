
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include "elab_pin.h"
#include "elab_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("EDF_PIN")

/* public functions --------------------------------------------------------- */
/**
  * @brief  eLab pin initialization.
  * @param  me      this pointer
  * @param  name    pin's name.
  * @param  mode    pin's mode.
  * @retval None
  */
void elab_pin_register(elab_pin_t * const me,
                        const char *name,
                        const elab_pin_ops_t *ops,
                        void *user_data)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    elab_assert(ops != NULL);

    elab_device_attr_t attr =
    {
        .name = (char *)name,
        .sole = true,
        .type = ELAB_DEVICE_PIN,
    };
    elab_device_register(&me->super, &attr);
    me->super.user_data = user_data;
    me->super.ops = NULL;

    me->ops = ops;
    me->ops->init(me);
    me->status = me->ops->get_status(me);
}

/**
  * @brief  eLab pin's status getting function.
  * @param  me      this pointer
  * @retval The pin's status.
  */
void elab_pin_set_mode(elab_device_t * const me, uint8_t mode)
{
    elab_assert(me != NULL);

    elab_pin_t *pin = (elab_pin_t *)me;
    if (pin->mode != mode)
    {
        pin->ops->set_mode(pin, mode);
        pin->mode = mode;
    }
}


/**
  * @brief  eLab pin's status getting function.
  * @param  me      this pointer
  * @retval The pin's status.
  */
bool elab_pin_get_status(elab_device_t *const me)
{
    elab_assert(me != NULL);
    elab_pin_t *pin = (elab_pin_t *)me;

    pin->status = pin->ops->get_status(pin);

    return pin->status;
}

/**
  * @brief  eLab pin's status turning on function.
  * @param  me      this pointer
  * @param  status  the input pin status.
  * @retval None.
  */
void elab_pin_set_status(elab_device_t *const me, bool status)
{
    elab_assert(me != NULL);

    elab_pin_t *pin = (elab_pin_t *)me;
    elab_assert(pin->mode == PIN_MODE_OUTPUT_PP || pin->mode == PIN_MODE_OUTPUT_OD);
    
    if (status != pin->status)
    {
        pin->ops->set_status(pin, status);
        elab_pin_get_status(me);
        elab_assert(pin->status == status);
    }
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
