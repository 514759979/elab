/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef EIO_PWM_H
#define EIO_PWM_H

/* includes ----------------------------------------------------------------- */
#include <stdbool.h>
#include "stm32g0xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public typedef ----------------------------------------------------------- */
typedef struct eio_pwm_data
{
    GPIO_TypeDef *gpio_x;
    uint16_t pin;
} eio_pwm_data_t;

typedef struct eio_pwm
{
    eio_pwm_data_t data;
    enum pin_mode mode;
    bool status;
} eio_pwm_t;

/* public functions --------------------------------------------------------- */
void eio_pwm_init(eio_pwm_t * const me, const char *name);
uint8_t eio_pwm_get_duty(eio_pwm_t * const me);
void eio_pwm_set_duty(eio_pwm_t * const me, uint8_t duty);

#ifdef __cplusplus
}
#endif

#endif

/* ----------------------------- end of file -------------------------------- */
