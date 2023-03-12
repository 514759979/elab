
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdlib.h>
#include "eio_pin.h"
#include "elab.h"

#ifdef __cplusplus
extern "C" {
#endif

/* private config ----------------------------------------------------------- */
#define LED_POLL_PERIOD_MS                  (100)

/* private variables -------------------------------------------------------- */
eio_pin_t *led = NULL;

/* includes ----------------------------------------------------------------- */
/**
  * @brief  LED initialization.
  * @retval None
  */
void led_init(void)
{
    led = eio_pin_find("LED1");
}
INIT_COMPONENT_EXPORT(led_init);

/**
  * @brief  LED polling function.
  * @retval None
  */
void led_poll(void)
{
    if ((elab_time_ms() % 1000) < 500)
    {
        eio_pin_set_status(led, true);
    }
    else
    {
        eio_pin_set_status(led, false);
    }
}
POLL_EXPORT(led_poll, LED_POLL_PERIOD_MS);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
