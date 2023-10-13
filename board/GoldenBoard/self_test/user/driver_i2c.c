#include "driver_spi.h"
#include "stm32g0xx_hal.h"
#include "elab_export.h"
#include "elab_common.h"

I2C_HandleTypeDef hi2c1;

    uint8_t byte_w[8] ={
        1, 2,3,4,5,6,7,8
    };

uint8_t byte_r[8];
extern HAL_StatusTypeDef status;

void Error_Handler(void);
    
void driver_i2c_init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x00400B27;
    hi2c1.Init.OwnAddress1 = 0x3f;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Analogue filter 
     */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Digital filter 
     */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
    {
        Error_Handler();
    }
    /** I2C Fast mode Plus enable 
     */
    HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C1);

    HAL_NVIC_SetPriority(I2C1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_IRQn);


    status = HAL_I2C_Mem_Write(&hi2c1, 0xA0, 0, 2, byte_w, 8, 100);
    status = HAL_I2C_Mem_Read(&hi2c1, 0xA0, 0, 2, byte_r, 8, 100);
}

INIT_IO_DRIVER_EXPORT(driver_i2c_init);

void Error_Handler(void)
{

}
