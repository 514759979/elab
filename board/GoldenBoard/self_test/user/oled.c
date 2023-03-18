

#include "oled.h"
#include "eio_object.h"
#include "eio_pin.h"
#include "driver_spi.h"
#include "elab_export.h"
#include "stm32g0xx_hal.h"

static eio_object_t *oled_dc = NULL;
static eio_object_t *oled_cs = NULL;
static eio_object_t *oled_rst = NULL;

static void _write_byte(uint8_t byte, bool cmd);


void oled_init(void)
{
    oled_dc = eio_find("OLedDc");
    oled_cs = eio_find("OLedCs");
    oled_rst = eio_find("OLedRst");
    
    HAL_Delay(100);
    
    eio_pin_set_status(oled_rst, false);
    HAL_Delay(10);
    eio_pin_set_status(oled_rst, true);

    _write_byte(0xAE, true);
    _write_byte(0x00, true);    //---set low column address
    _write_byte(0x10, true);    //---set high column address
    _write_byte(0x40, true);    //--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    _write_byte(0x81, true);    //--set contrast control register
    _write_byte(0xCF, true);    // Set SEG Output Current Brightness
    _write_byte(0xA1, true);    //--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
    _write_byte(0xC8, true);    //Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
    _write_byte(0xA6, true);    //--set normal display
    _write_byte(0xA8, true);    //--set multiplex ratio(1 to 64)
    _write_byte(0x3f, true);    //--1/64 duty
    _write_byte(0xD3, true);    //-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    _write_byte(0x00, true);    //-not offset
    _write_byte(0xd5, true);    //--set display clock divide ratio/oscillator frequency
    _write_byte(0x80, true);    //--set divide ratio, Set Clock as 100 Frames/Sec
    _write_byte(0xD9, true);    //--set pre-charge period
    _write_byte(0xF1, true);    //Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    _write_byte(0xDA, true);    // --set com pins hardware configuration
    _write_byte(0x12, true);
    _write_byte(0xDB, true);    // --set vcomh
    _write_byte(0x40, true);    // Set VCOM Deselect Level
    _write_byte(0x20, true);    // -Set Page Addressing Mode (0x00/0x01/0x02)
    _write_byte(0x02, true);    //
    _write_byte(0x8D, true);    // --set Charge Pump enable/disable
    _write_byte(0x14, true);    // --set(0x10) disable
    _write_byte(0xA4, true);    // Disable Entire Display On (0xa4/0xa5)
    _write_byte(0xA6, true);    // Disable Inverse Display On (0xa6/a7) 
    _write_byte(0xAF, true);    // --turn on oled panel
    
    oled_clear();
    oled_set_pos(4, 4, true);
    oled_open();
}

INIT_DEV_EXPORT(oled_init);

void oled_open(void)
{
    _write_byte(0X8D, true);    // Set DCDC命令
    _write_byte(0X14, true);    // DCDC ON
    _write_byte(0XAF, true);    // DISPLAY ON
}

void oled_close(void)
{
    _write_byte(0X8D, true);    // Set DCDC命令
    _write_byte(0X10, true);    // DCDC ON
    _write_byte(0XAE, true);    // DISPLAY OFF
}

void oled_set_pos(uint8_t x, uint8_t y, bool status)
{
    _write_byte((0xb0 + y), true);
    _write_byte(((x & 0xf0) >> 4)|0x10, true);
    _write_byte((x & 0x0f) | 0x01, true); 
}

void oled_clear(void)
{
    for (uint8_t i = 0; i < 8; i ++)
    {  
        _write_byte(0xb0 + i, true);    //设置页地址（0~7）
        _write_byte(0x00, true);      //设置显示位置—列低地址
        _write_byte(0x10, true);      //设置显示位置—列高地址   
        for (uint8_t n = 0; n < 128; n ++)
        {
            _write_byte(1, false);
        }
    }
}


static void _write_byte(uint8_t byte, bool cmd)
{
    eio_pin_set_status(oled_dc, cmd ? true : false);
    eio_pin_set_status(oled_cs, false);
    driver_spi_send(&byte, 1);
    eio_pin_set_status(oled_dc, true);
    eio_pin_set_status(oled_cs, true);
}
