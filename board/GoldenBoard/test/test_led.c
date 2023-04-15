
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "shell.h"
#include "elab_pin.h"

#ifdef __cplusplus
extern "C" {
#endif

/* private functions -------------------------------------------------------- */
/**
  * @brief Testing function for LED.
  * @retval None
  */
static int32_t test_func_led(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    elab_device_t *led = NULL;

    if (argc != 3)
    {
        ret = -1;
        goto exit;
    }

    led = elab_device_find(argv[1]);
    if (led == NULL)
    {
        ret = -2;
        goto exit;
    }

    if (strcmp(argv[2], "0") == 0)
    {
        elab_pin_set_status(led, false);
    }
    else if (strcmp(argv[2], "1") == 0)
    {
        elab_pin_set_status(led, true);
    }
    else
    {
        ret = -3;
        goto exit;
    }
    
    printf("test_func_led %s duty %s.\r\n", argv[1], argv[2]);

exit:
    return ret;
}

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_led,
                    test_func_led,
                    LED testing function);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
