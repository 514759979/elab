/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include "drv_button.h"
#include "drv_util.h"
#include "elab/edf/normal/elab_adc.h"
#include "elab/common/elab_assert.h"
#include "elab/common/elab_def.h"
#include "elab/common/elab_export.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("DriverButtonAdc");

/* private function prototype ----------------------------------------------- */
static bool _is_pressed(elab_button_t *const me);

/* private variables -------------------------------------------------------- */
static elab_button_ops_t button_driver_ops =
{
    .is_pressed = _is_pressed,
};

/* public functions --------------------------------------------------------- */
void elab_driver_button_adc_init(elab_button_driver_t *me,
                                    const char *name,
                                    const char *dev_name,
                                    float voltage_min,
                                    float voltage_max)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    elab_assert(dev_name != NULL);
    assert_name(elab_device_valid(dev_name), dev_name);

    me->dev_name = dev_name;
    me->voltage_min = voltage_min;
    me->voltage_max = voltage_max;

    elab_button_register(&me->device, name, &button_driver_ops, me);
}

/* private functions -------------------------------------------------------- */
/**
  * @brief  The PIN driver initialization function.
  * @param  me  PIN device handle.
  * @retval None.
  */
static bool _is_pressed(elab_button_t *const me)
{
    elab_assert(me != NULL);

    elab_button_driver_t *driver = (elab_button_driver_t *)me->super.user_data;
    elab_device_t *adc = elab_device_find(driver->dev_name);
    elab_assert(adc != NULL);

    float value = elab_adc_get_value(adc);
    return (value >= driver->voltage_min && value <= driver->voltage_max);
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
