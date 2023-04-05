#include "io_ex.h"
#include "stm32g0xx_hal.h"
#include "elab_export.h"
#include "elab_common.h"

#define I2C_ADDR_IO_EX                  0x38

uint8_t io_config_out = 0;
uint8_t out_status = 0;
uint8_t out_status_r = 0;
extern I2C_HandleTypeDef hi2c1;
HAL_StatusTypeDef status;

void io_ex_init(void)
{
    uint8_t polarity = 0;
    uint8_t config = !io_config_out;
    status = HAL_I2C_Mem_Write(&hi2c1, I2C_ADDR_IO_EX, 2, 1, &polarity, 1, 100);
    status = HAL_I2C_Mem_Write(&hi2c1, I2C_ADDR_IO_EX, 3, 1, &config, 1, 100);
    
    io_ex_set_out(2, true);
    io_ex_set_out(3, true);
}
INIT_DEV_EXPORT(io_ex_init);

void io_ex_set_out(uint8_t id, bool output)
{
    io_config_out |= (1 << id);

    uint8_t config = !io_config_out;
    status = HAL_I2C_Mem_Write(&hi2c1, I2C_ADDR_IO_EX, 3, 1, &config, 1, 100);
}

void io_ex_set_status(uint8_t id, bool status)
{
    if (io_config_out & (1 << id))
    {
        if (status)
        {
            out_status |= (1 << id);
        }
        else
        {
            out_status &=~ (1 << id);
        }
        
        status = HAL_I2C_Mem_Write(&hi2c1, I2C_ADDR_IO_EX, 1, 1, &out_status, 1, 100);
        status = HAL_I2C_Mem_Read(&hi2c1, I2C_ADDR_IO_EX, 1, 1, &out_status_r, 1, 100);
    }
}

bool io_ex_get_status(uint8_t id)
{
    uint8_t in_status = 0;
    status = HAL_I2C_Mem_Read(&hi2c1, I2C_ADDR_IO_EX, 0, 1, &in_status, 1, 100);

    return (in_status & (1 << id)) ? true : false;
}

void io_ex_poll(void)
{
    if ((elab_time_ms() % 1000) < 500)
    {
        io_ex_set_status(2, true);
        io_ex_set_status(3, false);
    }
    else
    {
        io_ex_set_status(2, false);
        io_ex_set_status(3, true);
    }
}

POLL_EXPORT(io_ex_poll, 100);
