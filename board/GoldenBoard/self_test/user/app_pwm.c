
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdlib.h>
#include "eio_object.h"
#include "eio_pwm.h"
#include "elab.h"
#include "driver_adc.h"
#include "stm32g0xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* private config ----------------------------------------------------------- */
#define LED_POLL_PERIOD_MS                  (5)

/* private variables -------------------------------------------------------- */
eio_object_t *led3 = NULL;
eio_object_t *led4 = NULL;

uint32_t buff_adc[16];

extern HAL_StatusTypeDef status;
extern ADC_HandleTypeDef hadc1;

float voltage = 0.0;
float voltage_get = 0.0;

/* includes ----------------------------------------------------------------- */
/**
  * @brief  LED initialization.
  * @retval None
  */
void pwm_init(void)
{
    led3 = eio_find("adc_in");
    elab_assert(led3 != NULL);
    
    adc_driver_init();
    while (1)
    {
        uint8_t duty = (uint8_t)(voltage / 3.3 * 100.0);
        eio_pwm_set_duty(led3, duty);
        status = HAL_ADC_Start(&hadc1);
        status = HAL_ADC_PollForConversion(&hadc1, 100);
        voltage_get = (float)HAL_ADC_GetValue(&hadc1) * 3.300 / 4096.0;
    }
    
//    led4 = eio_find("pwmled4");
//    elab_assert(led4 != NULL);
}
//INIT_COMPONENT_EXPORT(pwm_init);

/**
  * @brief  LED polling function.
  * @retval None
  */
void pwm_poll(void)
{
    uint16_t ms = elab_time_ms() % 1000;
    uint8_t duty_ratio;
    if (ms <= 500)
    {
        duty_ratio = ms / 5;
    }
    else
    {
        duty_ratio = (1000 - ms) / 5;
    }

    eio_pwm_set_duty(led3, duty_ratio);
    eio_pwm_set_duty(led4, duty_ratio);
}
//POLL_EXPORT(pwm_poll, LED_POLL_PERIOD_MS);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
