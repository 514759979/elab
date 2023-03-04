/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include "elab_common.h"
#include "elab_export.h"
#include "shell.h"

/* private config ----------------------------------------------------------- */
#define SHELL_POLL_PERIOD_MS                (10)
#define SHELL_BUFFER_SIZE                   (512)

/* private variables -------------------------------------------------------- */
static Shell shell_uart;
static char shell_uart_buffer[SHELL_BUFFER_SIZE];
static uint32_t time_ms = 0;

/* private functions -------------------------------------------------------- */
/**
  * @brief  Shell uart initialization.
  * @retval None
  */
static void shell_uart_init(void)
{
    elab_debug_uart_init(115200);

    shell_uart.read = (int16_t (*)(char *, uint16_t))elab_debug_uart_receive;
    shell_uart.write = (int16_t (*)(char *, uint16_t))elab_debug_uart_send;
    shellInit(&shell_uart, shell_uart_buffer, SHELL_BUFFER_SIZE);

    time_ms = elab_time_ms();
}
INIT_BSP_EXPORT(shell_uart_init);

/**
  * @brief  Shell polling function.
  * @retval None
  */
static void shell_poll(void)
{
    char byte;
    while (elab_time_ms() - time_ms >= SHELL_POLL_PERIOD_MS)
    {
        time_ms += SHELL_POLL_PERIOD_MS;

        while (shell_uart.read && shell_uart.read(&byte, 1) == 1)
        {
            shellHandler(&shell_uart, byte);
        }
    }
}
POLL_EXPORT(shell_poll, 10);

/* ----------------------------- end of file -------------------------------- */