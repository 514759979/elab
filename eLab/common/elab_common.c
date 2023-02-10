/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include "elab.h"

/* public function ---------------------------------------------------------- */
/**
  * @brief  eLab debug uart weak initialization function.
  * @param  baudrate    The uart port baudrate.
  * @retval None
  */
ELAB_WEAK void elab_debug_uart_init(uint32_t baudrate)
{
    (void)baudrate;
}

ELAB_WEAK int16_t elab_debug_uart_send(void *buffer, uint16_t size)
{
    (void)buffer;
    (void)size;

    return 0;
}

ELAB_WEAK int16_t elab_debug_uart_receive(void *buffer, uint16_t size)
{
    (void)buffer;
    (void)size;

    return 0;
}

ELAB_WEAK void elab_debug_uart_buffer_clear(void)
{
}

/* ----------------------------- end of file -------------------------------- */
