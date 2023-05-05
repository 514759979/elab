
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef SIMU_IO_DRIVER_SERIAL_H
#define SIMU_IO_DRIVER_SERIAL_H

/* include ------------------------------------------------------------------ */
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* public define ------------------------------------------------------------ */
#define SIMU_SERAIL_SLAVE_NUM_MAX           (16)
#define SIMU_SERIAL_BUFFER_MAX              (1024)

/* public function ---------------------------------------------------------- */
void simu_serial_new(const char *name);
void simu_serial_new_pair(const char *name_one, const char *name_two);
void simu_serial_make_rx_data(const char *name, void *buffer, uint32_t size);
void simu_serial_make_rx_data_delay(const char *name,
                                    void *buffer, uint32_t size,
                                    uint32_t delay_ms);
uint32_t simu_serial_read_tx_data(const char *name,
                                    void *buffer, uint32_t size,
                                    uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif  /* SIMU_IO_DRIVER_SERIAL_H */

/* ----------------------------- end of file -------------------------------- */
