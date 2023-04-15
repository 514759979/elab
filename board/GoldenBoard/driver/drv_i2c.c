
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include "elab_i2c.h"
#include "elab_assert.h"
#include "elab_def.h"
#include "elab_export.h"
#include "stm32g0xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("DriverI2C")

/* public typedef ----------------------------------------------------------- */
enum DRIVER_PIN_SPI
{
    DRIVER_PIN_I2C_SCK = 0,
    DRIVER_PIN_I2C_SDA,

    DRIVER_PIN_I2C_MAX
};

/* Public types ------------------------------------------------------------- */
typedef struct driver_pin_data
{
    GPIO_TypeDef *port;
    uint16_t pin;
} driver_pin_data_t;

typedef struct driver_spi_bus_data
{
    osSemaphoreId_t sem;
} driver_i2c_bus_data_t;

typedef struct driver_spi_bus_info
{
    elab_i2c_bus_t *device;
    driver_i2c_bus_data_t *data;
    const char *name;
    I2C_HandleTypeDef *hi2c;
    driver_pin_data_t pin[DRIVER_PIN_I2C_MAX];
} driver_i2c_bus_info_t;

typedef struct driver_spi_info
{
    elab_i2c_t *device;
    const char *name;
    const char *bus_name;
    elab_i2c_config_t config;
} driver_i2c_info_t;

static elab_i2c_bus_t i2c_bus;
static driver_i2c_bus_data_t i2c_data;
static I2C_HandleTypeDef hi2c1;
static DMA_HandleTypeDef hdma_i2c1_tx;
static DMA_HandleTypeDef hdma_i2c1_rx;

static const driver_i2c_bus_info_t _i2c_bus_info[] =
{
    /* I2C1 GPIO Configuration: PB8, PB9 */
    {
        &i2c_bus, &i2c_data, "I2C", &hi2c1,
        {
            { GPIOB, GPIO_PIN_8, },
            { GPIOB, GPIO_PIN_9, },
        },
    },
};

static elab_i2c_t i2c_exio;
static elab_i2c_t i2c_eeprom;

static const driver_i2c_info_t _i2c_info[] =
{
    { 
        &i2c_exio,  "I2C_EXIO",  "I2C", { 400000, false, 0x37, },
    },
    {
        &i2c_eeprom, "I2C_EEPROM", "I2C", { 400000, false, 32000000, },
    },
};

int32_t _xfer(elab_i2c_bus_t *, uint16_t addr, elab_i2c_msg_t msg);
elab_err_t _config(elab_i2c_bus_t *, elab_i2c_bus_config_t *config);

static const elab_i2c_bus_ops_t i2c_bus_ops =
{
    .xfer = _xfer,
    .config = _config,
};

/* public functions --------------------------------------------------------- */
static void _i2c_dirver_init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    /* I2C1 init. */
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x00400B27;
    hi2c1.Init.OwnAddress1 = 0x3f;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8 |GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    for (uint32_t i = 0;
            i < sizeof(_i2c_bus_info) / sizeof(driver_i2c_bus_info_t); i ++)
    {
        _i2c_bus_info[i].data->sem = osSemaphoreNew(1, 0, NULL);
        elab_assert(_spi_bus_info[i].data->sem != NULL);

        /* Device registering. */
        elab_spi_bus_register(_i2c_bus_info[i].device,
                                _i2c_bus_info[i].name,
                                &i2c_bus_ops,
                                (void *)&_i2c_bus_info[i]);
    }
    
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA1_Channel2_3_IRQn interrupt configuration. */
    HAL_NVIC_SetPriority(DMA1_Ch4_7_DMAMUX1_OVR_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Ch4_7_DMAMUX1_OVR_IRQn);

    /* SPI1 DMA Init */
    /* SPI1_TX Init */
    hdma_spi1_tx.Instance = DMA1_Channel4;
    hdma_spi1_tx.Init.Request = DMA_REQUEST_I2C1_TX;
    hdma_spi1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi1_tx.Init.Mode = DMA_NORMAL;
    hdma_spi1_tx.Init.Priority = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma_i2c1_tx);

    __HAL_LINKDMA(&hi2c1, hdmatx, hdma_i2c1_tx);

    /* SPI1_RX Init */
    hdma_spi1_rx.Instance = DMA1_Channel5;
    hdma_spi1_rx.Init.Request = DMA_REQUEST_SPI1_RX;
    hdma_spi1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_spi1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi1_rx.Init.Mode = DMA_NORMAL;
    hdma_spi1_rx.Init.Priority = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&hdma_i2c1_rx);

    __HAL_LINKDMA(&hi2c1, hdmarx, hdma_i2c1_rx);

    for (uint32_t i = 0;
            i < sizeof(_spi_info) / sizeof(driver_i2c_info_t); i ++)
    {
        /* Device registering. */
        elab_spi_register(_spi_info[i].device,
                            _spi_info[i].name,
                            _spi_info[i].bus_name,
                            &spi_ops,
                            _spi_info[i].config,
                            (void *)&_spi_info[i]);
    }

}
INIT_IO_DRIVER_EXPORT(_i2c_dirver_init);

elab_err_t _config(elab_i2c_t *const me, elab_i2c_config_t *config)
{
    elab_assert(me != NULL);
    elab_assert(config != NULL);
    elab_assert(config->data_width == 8);

    (void)me;
    (void)config;

    return ELAB_OK;
}

elab_err_t _xfer(elab_i2c_t * const me, elab_i2c_msg_t *msg)
{
    elab_assert(me != NULL);
    elab_assert(msg != NULL);
    elab_assert(msg->buff_send != NULL || msg->buff_recv != NULL);

    driver_spi_bus_info_t *spi_bus_info =
        (driver_spi_bus_info_t *)me->bus->super.user_data;

    if (msg->buff_send == NULL && msg->buff_recv != NULL)
    {
        HAL_SPI_Receive_DMA(spi_bus_info->hspi,
                                    (uint8_t *)msg->buff_recv, msg->size);
    }
    else if (msg->buff_send != NULL && msg->buff_recv == NULL)
    {
        HAL_SPI_Transmit_DMA(spi_bus_info->hspi,
                                    (uint8_t *)msg->buff_send, msg->size);
    }
    else if (msg->buff_send != NULL && msg->buff_recv != NULL)
    {
        HAL_SPI_TransmitReceive_DMA(spi_bus_info->hspi,
                                    (uint8_t *)msg->buff_send,
                                    (uint8_t *)msg->buff_recv, msg->size);
    }

    osStatus_t ret_os = osSemaphoreAcquire(spi_bus_info->data->sem, osWaitForever);
    elab_assert(ret_os == osOK);

    return ELAB_OK;
}

static osSemaphoreId_t _find_sem_from_spi(SPI_HandleTypeDef *hspi)
{
    osSemaphoreId_t ret_sem = NULL;

    for (uint32_t i = 0;
            i < sizeof(_spi_bus_info) / sizeof(driver_spi_bus_info_t); i ++)
    {
        if (_spi_bus_info[i].hspi == hspi)
        {
            ret_sem = _spi_bus_info[i].data->sem;
            break;
        }
    }
    elab_assert(ret_sem == NULL);

    return ret_sem;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    osSemaphoreRelease(_find_sem_from_spi(hspi));
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    osSemaphoreRelease(_find_sem_from_spi(hspi));
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    osSemaphoreRelease(_find_sem_from_spi(hspi));
}

/**
  * @brief This function handles DMA1 channel 2 and channel 3 interrupts.
  */
void DMA1_Channel2_3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_spi1_rx);
    HAL_DMA_IRQHandler(&hdma_spi1_tx);
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
