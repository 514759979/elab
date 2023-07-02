
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "eLab/3rd/Shell/shell.h"
#include "oled.h"

#ifdef __cplusplus
extern "C" {
#endif

/* private functions -------------------------------------------------------- */
/**
  * @brief OLED opening testing function.
  * @retval None
  */
static int32_t test_func_oled_open(int32_t argc, char *argv[])
{
    int32_t ret = 0;

    if (argc != 1)
    {
        ret = -1;
        goto exit;
    }

    oled_open();
    printf("test_func_oled_open end.\r\n");

exit:
    return ret;
}

/**
  * @brief OLED closing testing function.
  * @retval None
  */
static int32_t test_func_oled_close(int32_t argc, char *argv[])
{
    int32_t ret = 0;

    if (argc != 1)
    {
        ret = -1;
        goto exit;
    }

    oled_close();
    printf("test_func_oled_close end.\r\n");

exit:
    return ret;
}

/**
  * @brief OLED clear testing function.
  * @retval None
  */
static int32_t test_func_oled_clear(int32_t argc, char *argv[])
{
    int32_t ret = 0;

    if (argc != 1)
    {
        ret = -1;
        goto exit;
    }

    oled_close();
    printf("test_func_oled_clear end.\r\n");

exit:
    return ret;
}

/**
  * @brief OLED closing testing function.
  * @retval None
  */
static int32_t test_func_oled_set_value(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    uint32_t x, y, value;

    if (argc != 4)
    {
        ret = -1;
        goto exit;
    }

    x = atoi(argv[1]);
    y = atoi(argv[2]);
    value = atoi(argv[3]);

    oled_set_value((uint8_t)x, (uint8_t)y, (uint8_t)value);
    printf("test_func_oled_set_value end.\r\n");

exit:
    return ret;
}

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_oled_open,
                    test_func_oled_open,
                    OLED open testing);

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_oled_close,
                    test_func_oled_close,
                    OLED close testing);

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_oled_clear,
                    test_func_oled_clear,
                    OLED clear testing);

/**
  * @brief  Test shell command export
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_oled_set_value,
                    test_func_oled_set_value,
                    OLED set value testing);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
