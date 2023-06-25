
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include <stdlib.h>
#include "eLab/3rd/Shell/shell.h"
#include "eLab/common/elab_assert.h"
#include "eLab/common/elab_export.h"
#include "eLab/common/elab_common.h"
#include "eLab/RTOS/cmsis_os.h"

ELAB_TAG("ShellExport");

/* private defines ---------------------------------------------------------- */
#define ELAB_SHELL_BUFF_SIZE                    (2048)

typedef struct esh_key_info
{
    uint8_t key_id;
    uint8_t count;
    uint8_t value[5];
} esh_key_info_t;

/* private variables -------------------------------------------------------- */
static const osThreadAttr_t thread_sh_attr =
{
    .name = "shell",
    .stack_size = 2048,
    .priority = (osPriority_t)osPriorityBelowNormal,
};

static Shell shell;
static uint8_t buff_shell[ELAB_SHELL_BUFF_SIZE];

/* private function prototype ----------------------------------------------- */
static uint8_t _func_key_parser(uint8_t *key, uint8_t count);

/* public function ---------------------------------------------------------- */
/**
  * @brief  Shell exporting fucntion
  * @param  None
  * @retval None
  */
static void shell_init(void)
{
    shell.read = (int16_t (*)(char *, uint16_t))elab_debug_uart_receive;
    shell.write = (int16_t (*)(char *, uint16_t))elab_debug_uart_send;
    shellInit(&shell, buff_shell, ELAB_SHELL_BUFF_SIZE);
    shell.backsapceMode = SHELL_BACKSPACE_MODE_BACK_SPACE_BACK;

    osThreadId_t thread = osThreadNew(shellTask, (void*)&shell, &thread_sh_attr);
    elab_assert(thread != NULL);
}

/**
  * @brief  Export pin device board level init and exit function
  */
INIT_EXPORT(shell_init, 0);

/* ----------------------------- end of file -------------------------------- */
