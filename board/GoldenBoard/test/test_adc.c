
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "elab/3rd/Shell/shell.h"
#include "elab/edf/normal/elab_adc.h"

#define TAG                                 "AdcTest"
#include "elab/common/elab_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/* private functions -------------------------------------------------------- */
/**
  * @brief Testing function for LED.
  * @retval None
  */
static int32_t test_func_adc(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    elab_device_t *adc = NULL;
    float value = 0.0;

    if (argc != 2)
    {
        ret = -1;
        goto exit;
    }

    adc = elab_device_find(argv[1]);
    if (adc == NULL)
    {
        elog_error("Find device %s failure.", argv[1]);
        ret = -2;
        goto exit;
    }

    value = elab_adc_get_value(adc);
    elog_debug("ADC %s 's value is %f.", argv[1], value);

exit:
    return ret;
}

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_adc,
                    test_func_adc,
                    ADC testing function);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
