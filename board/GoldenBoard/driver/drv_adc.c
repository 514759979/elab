
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include "elab/edf/normal/elab_adc.h"
#include "elab/common/elab_assert.h"
#include "elab/common/elab_def.h"
#include "elab/common/elab_export.h"
#include "stm32g0xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("DriverADC");

/* public typedef ----------------------------------------------------------- */
typedef struct elab_adc_data
{
    elab_adc_t *device;
    const char *name;
    GPIO_TypeDef *gpio_x;
    uint16_t pin;
} elab_adc_data_t;

/* private function prototype ----------------------------------------------- */
static uint32_t _get_value(elab_adc_t * const me);
static void _init(elab_adc_t * const me);

/* private variables -------------------------------------------------------- */
static elab_adc_t adc_a00;
static elab_adc_t adc_a01;

static ADC_HandleTypeDef hadc1;

static const elab_adc_ops_t _driver_ops =
{
    .get_value = _get_value,
};

static elab_adc_data_t _adc_driver_data[] =
{
    { &adc_a00, "PwmAdc", GPIOA, GPIO_PIN_0, },
    { &adc_a01, "ButtonAdc", GPIOC, GPIO_PIN_9, },
};

/* public functions --------------------------------------------------------- */
static void _adc_dirver_init(void)
{
    HAL_StatusTypeDef ret = HAL_OK;
    
    __HAL_RCC_ADC_CLK_ENABLE();
    
    /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
     */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.LowPowerAutoPowerOff = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_1CYCLE_5;
    hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_1CYCLE_5;
    hadc1.Init.OversamplingMode = DISABLE;
    hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
    ret = HAL_ADC_Init(&hadc1);
    elab_assert(ret == HAL_OK);

    /** Configure Regular Channel */
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_2;
    ret = HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    elab_assert(ret == HAL_OK);
    
    for (uint32_t i = 0;
            i < sizeof(_adc_driver_data) / sizeof(elab_adc_data_t); i ++)
    {
        _adc_driver_data[i].device->super.user_data = &_adc_driver_data[i];
        
        /* Enable the clock. */
        if (_adc_driver_data[i].gpio_x == GPIOA)
        {
            __HAL_RCC_GPIOA_CLK_ENABLE();
        }
        else if (_adc_driver_data[i].gpio_x == GPIOB)
        {
            __HAL_RCC_GPIOB_CLK_ENABLE();
        }
        else if (_adc_driver_data[i].gpio_x == GPIOC)
        {
            __HAL_RCC_GPIOC_CLK_ENABLE();
        }
        else if (_adc_driver_data[i].gpio_x == GPIOD)
        {
            __HAL_RCC_GPIOD_CLK_ENABLE();
        }
        
        _init(_adc_driver_data[i].device);

        /* Device registering. */
        elab_adc_register(_adc_driver_data[i].device,
                            _adc_driver_data[i].name,
                            &_driver_ops,
                            &_adc_driver_data[i]);
    }

    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADC_Start(&hadc1);
}
INIT_EXPORT(_adc_dirver_init, 1);

/* private functions -------------------------------------------------------- */
/**
  * @brief  The PIN driver initialization function.
  * @param  me  PIN device handle.
  * @retval None.
  */
static void _init(elab_adc_t * const me)
{
    elab_adc_data_t *_data = (elab_adc_data_t *)me->super.user_data;

    /* Configure GPIO pin. */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = _data->pin;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(_data->gpio_x, &GPIO_InitStruct);
}

/**
  * @brief  The ADC driver get_value function.
  * @param  me  PIN device handle.
  * @retval ADC value.
  */
static uint32_t _get_value(elab_adc_t * const me)
{
    elab_assert(me != NULL);

    HAL_StatusTypeDef ret = HAL_OK;

    ret = HAL_ADC_Start(&hadc1);
    elab_assert(ret == HAL_OK);
    ret = HAL_ADC_PollForConversion(&hadc1, 100);
    elab_assert(ret == HAL_OK);
    uint32_t value = HAL_ADC_GetValue(&hadc1);
    ret = HAL_ADC_Stop(&hadc1);
    elab_assert(ret == HAL_OK);

    return value;
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
