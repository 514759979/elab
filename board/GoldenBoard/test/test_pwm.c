
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "elab_pwm.h"

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
    int32_t duty = 0;

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

    duty = atoi(argv[2]);
    if (duty < 0 || duty > 100)
    {
        ret = -3;
        goto exit;
    }
    
    elab_pwm_set_duty(led, duty);

exit:
    return ret;
}

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_pwmled,
                    test_func_led,
                    LED testing function);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
