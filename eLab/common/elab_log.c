/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include <stdio.h>
#include "elab_log.h"
#include "elab_assert.h"
#include "elab_common.h"
#include "cmsis_os.h"

/* private defines ---------------------------------------------------------- */
/* define printf color */
#define NONE                                        "\033[0;0m"
#define LIGHT_RED                                   "\033[1;31m"
#define YELLOW                                      "\033[0;33m"
#define LIGHT_BLUE                                  "\033[1;34m"
#define GREEN                                       "\033[0;32m"

/* private variables -------------------------------------------------------- */
static const osMutexAttr_t mutex_attr_elog =
{
    "mutex_elog", osMutexRecursive | osMutexPrioInherit, NULL, 0U 
};

/* public function prototype ------------------------------------------------ */
static uint8_t elog_level = ELOG_LEVEL_DEBUG;

#if (ELOG_OS_ENABLE != 0)
static osMutexId_t mutex_elog = NULL;
#endif

#if (ELOG_COLOR_ENABLE != 0)
static const char *const elog_color_table[ELOG_LEVEL_MAX] =
{
    NONE, LIGHT_RED, YELLOW, LIGHT_BLUE, GREEN,
};
#endif

static const char elog_level_lable[ELOG_LEVEL_MAX] =
{
    ' ', 'E', 'W', 'I', 'D',
};

/* public function ---------------------------------------------------------- */
void elog_level_set(uint8_t level)
{
    elog_level = level;
}

void _printf(const char *name, uint8_t level, const char * s_format, ...)
{
#if (ELOG_OS_ENABLE != 0)
    if (mutex_elog == NULL)
    {
        mutex_elog = osMutexNew(&mutex_attr_elog);
        /* TODO It's better not to user assert function here. */
        elab_assert(mutex_elog != NULL);
    }
    osMutexAcquire(mutex_elog, osWaitForever);
#endif

    if (elog_level >= level)
    {
#if (ELOG_COLOR_ENABLE == 0)
        printf("[%c/%s %u] ", elog_level_lable[level], name, elab_time_ms());
        printf("%s\n", s_format);
#else
        printf("%s[%c/%s %u] ", elog_color_table[level],
                                elog_level_lable[level], name, elab_time_ms());
        printf("%s%s\n", s_format, NONE);
#endif
    }
    
#if (ELOG_OS_ENABLE != 0)
    osMutexRelease(mutex_elog);
#endif
}

#if !defined(__x86_64__)
/* TODO Redirect printf. */
int _write(int file, char *ptr, int len)
{
    (void)file;
    (void)ptr;
    (void)len;

    return 0;
}
#endif

/* ----------------------------- end of file -------------------------------- */