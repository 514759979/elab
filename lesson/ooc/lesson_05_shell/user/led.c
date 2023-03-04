
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "eio_pin.h"
#include "elab.h"

#ifdef __cplusplus
extern "C" {
#endif

/* private config ----------------------------------------------------------- */
#define LED_POLL_PERIOD_MS                  (100)

/* private variables -------------------------------------------------------- */
extern eio_pin_t *led = NULL;
static eio_pin_t pin_led;

/* includes ----------------------------------------------------------------- */
/**
  * @brief  LED initialization.
  * @retval None
  */
void led_init(void)
{
    eio_pin_init(&pin_led, "C.08", PIN_MODE_OUTPUT);
    led = &pin_led;
}
INIT_IO_DRIVER_EXPORT(led_init);

/**
  * @brief  LED polling function.
  * @retval None
  */
void led_poll(void)
{
    if ((elab_time_ms() % 1000) < 500)
    {
        eio_pin_set_status(&pin_led, true);
    }
    else
    {
        eio_pin_set_status(&pin_led, false);
    }
}
POLL_EXPORT(led_poll, LED_POLL_PERIOD_MS);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
