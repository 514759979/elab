
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include "elab/edf/normal/elab_spi.h"
#include "elab/common/elab_assert.h"
#include "elab/common/elab_def.h"
#include "elab/common/elab_export.h"
#include "stm32g0xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("DriverSPI");

/* public typedef ----------------------------------------------------------- */
enum DRIVER_PIN_SPI
{
    DRIVER_PIN_SPI_SCK = 0,
    DRIVER_PIN_SPI_MISO,
    DRIVER_PIN_SPI_MOSI,

    DRIVER_PIN_SPI_MAX
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
} driver_spi_bus_data_t;

typedef struct driver_spi_bus_info
{
    elab_spi_bus_t *device;
    driver_spi_bus_data_t *data;
    const char *name;
    SPI_HandleTypeDef *hspi;
    driver_pin_data_t pin[DRIVER_PIN_SPI_MAX];
} driver_spi_bus_info_t;

typedef struct driver_spi_info
{
    elab_spi_t *device;
    const char *name;
    const char *bus_name;
    GPIO_TypeDef *gpio_cs;
    uint16_t pin_cs;
    elab_spi_config_t config;
} driver_spi_info_t;

static elab_spi_bus_t spi_bus;
static driver_spi_bus_data_t spi_data;
static SPI_HandleTypeDef hspi1;
static DMA_HandleTypeDef hdma_spi1_tx;
static DMA_HandleTypeDef hdma_spi1_rx;

static const driver_spi_bus_info_t _spi_bus_info[] =
{
    /** SPI1 GPIO Configuration
        PA5     ------> SPI1_SCK
        PA6     ------> SPI1_MISO
        PA7     ------> SPI1_MOSI
    */
    {
        &spi_bus, &spi_data, "SPI", &hspi1,
        {
            { GPIOA, GPIO_PIN_5, },
            { GPIOA, GPIO_PIN_6, },
            { GPIOA, GPIO_PIN_7, },
        },
    },
};

static elab_spi_t spi_oled;
static elab_spi_t spi_flash;

static const driver_spi_info_t _spi_info[] =
{
    { 
        &spi_oled,  "SPI_OLED",  "SPI", GPIOA, GPIO_PIN_2,
        { ELAB_SPI_MODE_0, 8, 32000000, },
    },
    {
        &spi_flash, "SPI_FLASH", "SPI", GPIOD, GPIO_PIN_2,
        { ELAB_SPI_MODE_0, 8, 32000000, },
    },
};

elab_err_t _config(elab_spi_t *const me, elab_spi_config_t *config);
elab_err_t _xfer(elab_spi_t * const me, elab_spi_msg_t *message);
void _enable(elab_spi_t * const me, bool status);

static const elab_spi_bus_ops_t spi_bus_ops =
{
    .config = _config,
    .xfer = _xfer,
};

static const elab_spi_ops_t spi_ops =
{
    .enable = _enable,
};

/* public functions --------------------------------------------------------- */
static void _spi_dirver_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();

    /* SPI1 init. */
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 7;
    hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    HAL_SPI_Init(&hspi1);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    for (uint32_t i = 0;
            i < sizeof(_spi_bus_info) / sizeof(driver_spi_bus_info_t); i ++)
    {
        _spi_bus_info[i].data->sem = osSemaphoreNew(1, 0, NULL);
        elab_assert(_spi_bus_info[i].data->sem != NULL);

        /* Device registering. */
        elab_spi_bus_register(_spi_bus_info[i].device,
                                _spi_bus_info[i].name,
                                &spi_bus_ops,
                                (void *)&_spi_bus_info[i]);
    }
    
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA1_Channel2_3_IRQn interrupt configuration. */
    HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

    /* SPI1 DMA Init */
    /* SPI1_TX Init */
    hdma_spi1_tx.Instance = DMA1_Channel3;
    hdma_spi1_tx.Init.Request = DMA_REQUEST_SPI1_TX;
    hdma_spi1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi1_tx.Init.Mode = DMA_NORMAL;
    hdma_spi1_tx.Init.Priority = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma_spi1_tx);

    __HAL_LINKDMA(&hspi1, hdmatx,hdma_spi1_tx);

    /* SPI1_RX Init */
    hdma_spi1_rx.Instance = DMA1_Channel2;
    hdma_spi1_rx.Init.Request = DMA_REQUEST_SPI1_RX;
    hdma_spi1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_spi1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi1_rx.Init.Mode = DMA_NORMAL;
    hdma_spi1_rx.Init.Priority = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&hdma_spi1_rx);

    __HAL_LINKDMA(&hspi1, hdmarx, hdma_spi1_rx);

    for (uint32_t i = 0;
            i < sizeof(_spi_info) / sizeof(driver_spi_info_t); i ++)
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
INIT_EXPORT(_spi_dirver_init, 1);

/* private functions -------------------------------------------------------- */
elab_err_t _config(elab_spi_t *const me, elab_spi_config_t *config)
{
    elab_assert(me != NULL);
    elab_assert(config != NULL);
    elab_assert(config->data_width == 8);

    (void)me;
    (void)config;

    return ELAB_OK;
}

elab_err_t _xfer(elab_spi_t * const me, elab_spi_msg_t *msg)
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

void _enable(elab_spi_t * const me, bool status)
{
    elab_assert(me != NULL);

    driver_spi_info_t *spi_data = (driver_spi_info_t *)me->super.user_data;

    GPIO_PinState pin_state = status ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(spi_data->gpio_cs, spi_data->pin_cs, pin_state);
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
    elab_assert(ret_sem != NULL);

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
