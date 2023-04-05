#ifndef DRIVER_SPI_H
#define DRIVER_SPI_H

#include <stdint.h>

void driver_spi_send(uint8_t *buffer, uint32_t size);
void driver_spi_send_recv(uint8_t *buffer_tx, uint8_t *buffer_rx, uint32_t size);
void driver_spi_recv(uint8_t *buffer_rx, uint32_t size);

#endif
