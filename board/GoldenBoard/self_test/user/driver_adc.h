#ifndef DRIVER_ADC_H
#define DRIVER_ADC_H

#include <stdint.h>

enum adc_id
{
    ADC_ID_DAC = 0,
    ADC_ID_BUTTON,
};

void adc_driver_init(void);
void driver_adc_get_value(uint8_t adc_id);
void driver_adc_start(uint32_t *buffer, uint32_t size);

#endif
