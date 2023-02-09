/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include "elab.h"
#include <sys/time.h>

static uint64_t time_init = UINT64_MAX;

/* public function ---------------------------------------------------------- */
/**
  * @brief  eLab initialization exporting function.
  * @param  level       init level.
  * @retval None
  */
uint32_t elab_time_ms(void)
{
    struct timeval tt;
    gettimeofday(&tt, NULL);
    uint64_t time_current = (tt.tv_sec * 1000) + (tt.tv_usec / 1000);

    if (time_init == UINT64_MAX)
    {
        time_init = time_current;
    }

    return (uint32_t)(time_current - time_init);
    return 0;
}

/* ----------------------------- end of file -------------------------------- */
