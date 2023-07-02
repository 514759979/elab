#include "elab_led.h"
#include "../../common/elab_assert.h"

ELAB_TAG("EdfLed");

#define ELAB_LED_POLL_PEROID                    (50)

/* private variables -------------------------------------------------------- */
enum elab_led_mode
{
    ELAB_LED_MODE_NULL = 0,
    ELAB_LED_MODE_TOGGLE,
    ELAB_LED_MODE_VALUE,
};

/* Private variables ---------------------------------------------------------*/
static const elab_dev_ops_t _led_ops =
{
    .enable = NULL,
    .read = NULL,
    .write = NULL,
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
    .poll = NULL,
#endif
};

static const osTimerAttr_t timer_attr_led =
{
    .name = "led_timer",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0,
};

/* public function ---------------------------------------------------------- */
void elab_led_init(elab_led_t *const me, const char *name,
                        elab_led_ops_t *ops, void *user_data)
{
    osStatus_t ret_os = osOK;
    elab_err_t ret = ELAB_OK;

    /* Newly establish the timer and start it. */
    me->timer = osTimerNew(_timer_func, osTimerPeriodic, me, &timer_attr_led);
    assert_name(me->timer != NULL, name);
    ret_os = osTimerStart(me->timer, ELAB_LED_POLL_PEROID);
    elab_assert(ret_os == osOK);

    /* Set the data of the device. */
    me->super.ops = &_led_ops;
    me->super.user_data = user_data;
    me->mode = ELAB_LED_MODE_NULL;
    ret = me->ops->set_status(me, false);
    me->status = false;
    elab_assert(ret == ELAB_OK);

    /* Register to device manager. */
    elab_device_attr_t attr_led =
    {
        .name = name,
        .sole = false,
        .type = ELAB_DEVICE_UNKNOWN,
    };
    elab_device_register(&me->super, &attr_led);
}

void elab_led_set_status(elab_device_t *const me, bool status)
{
    elab_assert(me != NULL);

    elab_led_t *led = ELAB_LED_CAST(me);

    led->mode = ELAB_LED_MODE_NULL;
}

void elab_led_toggle(elab_device_t *const me, uint32_t period_ms)
{
    elab_assert(me != NULL);
    
    elab_led_t *led = ELAB_LED_CAST(me);
    elab_assert(led->period_ms >= ELAB_LED_POLL_PEROID);

    if (led->mode != ELAB_LED_MODE_TOGGLE || led->period_ms != period_ms)
    {
        led->mode = ELAB_LED_MODE_TOGGLE;
        led->time_out = osKernelGetTickCount() + period_ms;
        led->period_ms = period_ms;
    }
}

/**
  * @brief  The led device value setting function.
  * @param  me      elab led device handle.
  * @param  value   The led device value.
  * @retval None.
  */
void elab_led_set_value(elab_device_t *const me, uint8_t value)
{
    elab_assert(me != NULL);

    elab_led_t *led = ELAB_LED_CAST(me);
    if (led->mode != ELAB_LED_MODE_VALUE || led->period_ms != value)
    {
        led->mode = ELAB_LED_MODE_VALUE;
        led->value = value;
        led->value_count = value;
    }
}

/* private function --------------------------------------------------------- */
/**
  * @brief  eLab LED polling timer function.
  * @param  argument    Timer function argument.
  * @retval None
  */
static void _timer_func(void *argument)
{
    elab_led_t *led = ELAB_LED_CAST(argument);

    /* When led is working in TOGGLE mode. */
    if (led->mode == ELAB_LED_MODE_TOGGLE)
    {
        if (osKernelGetTickCount() >= led->time_out)
        {
            led->status = !led->status;
            led->ops->set_status(led, led->status);
            led->time_out += led->period_ms;
        }
    }
    /* When led is working in VALUE mode. */
    else if (led->mode == ELAB_LED_MODE_VALUE)
    {

    }
}

/* ----------------------------- end of file -------------------------------- */
