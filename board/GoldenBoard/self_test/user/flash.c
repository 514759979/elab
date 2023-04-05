#include "flash.h"
#include "eio_object.h"
#include "eio_pin.h"
#include "driver_spi.h"
#include "elab_export.h"
#include "stm32g0xx_hal.h"

eio_object_t *flash_cs = NULL;
eio_object_t *oled_cs = NULL;

uint8_t byte_tx = 0x9f;
uint8_t byte_rx = 0;
uint8_t buff_id[5];
uint16_t flash_id = 0;

void flash_init(void)
{
    flash_cs = eio_find("FlashCS");
    oled_cs = eio_find("OLedCs");
    
    eio_pin_set_status(oled_cs, true);
    eio_pin_set_status(flash_cs, false);
    
    HAL_Delay(100);
    byte_tx = 0x90;
    driver_spi_send_recv(&byte_tx, &byte_rx, 1);
    driver_spi_recv(buff_id, 5);
    
    byte_tx = 0x90;
    driver_spi_send_recv(&byte_tx, &byte_rx, 1);
    byte_tx = 0x00;
    driver_spi_send_recv(&byte_tx, &byte_rx, 1);
    byte_tx = 0x00;
    driver_spi_send_recv(&byte_tx, &byte_rx, 1);
    byte_tx = 0x00;
    driver_spi_send_recv(&byte_tx, &byte_rx, 1);
    
    byte_tx = 0xff;
    driver_spi_send_recv(&byte_tx, &byte_rx, 1);
    flash_id |= (byte_rx << 8);
    byte_tx = 0xff;
    driver_spi_send_recv(&byte_tx, &byte_rx, 1);
    flash_id |= byte_rx;


    eio_pin_set_status(flash_cs, true);
}

INIT_DEV_EXPORT(flash_init);
