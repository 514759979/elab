#include "driver_adc.h"
#include "stm32g0xx_hal.h"
#include "elab_export.h"
#include "elab_common.h"

ADC_HandleTypeDef hadc1;

extern HAL_StatusTypeDef status;

uint32_t adc_value[16];

static void Error_Handler(void);
void driver_adc_start(uint32_t *buffer, uint32_t size);

void adc_driver_init(void)
{
    __HAL_RCC_ADC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    ADC_ChannelConfTypeDef sConfig = {0};
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }
    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_2;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
      Error_Handler();
    }



//    /* ADC1 DMA Init */
//    /* ADC1 Init */
//    hdma_adc1.Instance = DMA1_Channel1;
//    hdma_adc1.Init.Request = DMA_REQUEST_ADC1;
//    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
//    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
//    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
//    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
//    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
//    hdma_adc1.Init.Mode = DMA_CIRCULAR;
//    hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
//    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
//    {
//      Error_Handler();
//    }

//    __HAL_LINKDMA(&hadc1,DMA_Handle,hdma_adc1);

    /* ADC1 interrupt Init */
    HAL_NVIC_SetPriority(ADC1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC1_IRQn);

    
    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADC_Start(&hadc1);

}
//INIT_IO_DRIVER_EXPORT(adc_driver_init);

/**
  * @brief This function handles ADC1 interrupt.
  */
void ADC1_IRQHandler(void)
{
  /* USER CODE BEGIN ADC1_IRQn 0 */

  /* USER CODE END ADC1_IRQn 0 */
  HAL_ADC_IRQHandler(&hadc1);
  /* USER CODE BEGIN ADC1_IRQn 1 */

  /* USER CODE END ADC1_IRQn 1 */
}

void driver_adc_start(uint32_t *buffer, uint32_t size)
{

}

static void Error_Handler(void)
{
}
