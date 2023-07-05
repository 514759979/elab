
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "elab/3rd/Shell/shell.h"
#include "elab/common/elab_log.h"
#include "elab/edf/elab_device.h"
#include "export/rbc_export.h"

ELAB_TAG("MotorTest");

#ifdef __cplusplus
extern "C" {
#endif

/* private functions -------------------------------------------------------- */
/**
  * @brief Testing function for motor device.
  * @retval None
  */
static int32_t test_motor(int32_t argc, char *argv[])
{
    elab_err_t ret_motor = ELAB_OK;
    int32_t index = INT32_MAX;
    float speed = 100.0;

    if (argc != 3)
    {
        return -1;
    }

    index = atoi(argv[1]);
    if (!(index >= 0 && index <= 3))
    {
        return -2;
    }

    speed = (float)atoi(argv[2]);
    if (!(speed >= -50.0 && index <= 50.0))
    {
        return -3;
    }

    ret_motor = elab_motor_set_speed(motor[index], speed);
    if (ret_motor != ELAB_OK)
    {
        elog_error("elab_motor_set_speed error: %d.\n", (int32_t)ret_motor);
        return -4;
    }

    return 0;
}

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_motor,
                    test_motor,
                    motor testing function);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
