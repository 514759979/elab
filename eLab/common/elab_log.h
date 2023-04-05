/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ELAB_LOG_H_
#define ELAB_LOG_H_

/* includes ----------------------------------------------------------------- */
#include <stdint.h>

/* public config ------------------------------------------------------------ */
#define ELOG_COLOR_ENABLE                           (1)
#define ELOG_OS_ENABLE                              (1)

/* public defines ----------------------------------------------------------- */
#ifndef TAG
#define TAG                                         "eLog"
#endif

/* debug level */
enum elog_level_enum
{
    ELOG_LEVEL_ERROR = 1,
    ELOG_LEVEL_WARNING,
    ELOG_LEVEL_INFO,
    ELOG_LEVEL_DEBUG,

    ELOG_LEVEL_MAX,
};

/* public functions --------------------------------------------------------- */
void elog_level_set(uint8_t level);

#ifndef ELOG_DISABLE
void _printf(const char *tag, uint8_t level, const char * s_format, ...);

/* Enable error level debug message */
#define elog_error(...)     _printf(TAG, ELOG_LEVEL_ERROR, __VA_ARGS__)
#define elog_warn(...)      _printf(TAG, ELOG_LEVEL_WARNING, __VA_ARGS__)

/* Enable info level debug message */
#if ELOG_INFO_ENABLE
#define elog_info(...)      _printf(TAG, ELOG_LEVEL_INFO, __VA_ARGS__)
#else
#define elog_info(...)
#endif

/* Enable debug level debug message */
#if ELOG_DEBUG_ENABLE
#define elog_debug(...)     _printf(TAG, ELOG_LEVEL_DEBUG, __VA_ARGS__)
#else
#define elog_debug(...)
#endif

#else

#define elog_error(...)
#define elog_warn(...)
#define elog_info(...)
#define elog_debug(...)

#endif

#endif /* ELAB_LOG_H_ */

/* ----------------------------- end of file -------------------------------- */
