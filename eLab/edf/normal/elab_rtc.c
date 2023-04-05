/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_rtc.h"
#include "elab_assert.h"

#define TAG                             "elab_rtc"
#define ELOG_DEBUG_ENABLE               (1)
#define ELOG_INFO_ENABLE                (1)
#include "elab_log.h"

static elab_err_t _device_enable(elab_device_t *me, bool status)
{
    return ELAB_OK;
}

const static elab_dev_ops_t device_ops =
{
    .enable = _device_enable,
    .read = NULL,
    .write = NULL,
#if (ELAB_DEV_PALTFORM == 1)
    .poll = NULL,
#endif
};

void elab_rtc_register(elab_rtc_t *rtc, const char *name,
                        elab_rtc_ops_t *ops, void *user_data)
{
    elab_assert(rtc != NULL);
    elab_assert(name != NULL);
    elab_assert(ops != NULL);

    elab_device_t *device = &(rtc->super);

    device->ops = &device_ops;
    device->user_data = user_data;

    /* register a character device */
    elab_device_attr_t attr =
    {
        .name = name,
        .sole = true,
        .type = ELAB_DEVICE_RTC,
    };
    elab_device_register(device, &attr);
}

void elab_rtc_get_time(elab_device_t *me, elab_rtc_time_t *rtc_time)
{
    elab_assert(me != NULL);
    elab_assert(rtc_time != NULL);

    elab_rtc_t *rtc = (elab_rtc_t *)me;

    elab_err_t ret = rtc->ops->get_time(rtc, rtc_time);
    if (ret != ELAB_OK)
    {
        elog_error("RTC get time error. Error ID: %d.", (int32_t)ret);
    }
}

void elab_rtc_set_time(elab_device_t *me, const elab_rtc_time_t *rtc_time)
{
    elab_assert(me != NULL);
    elab_assert(rtc_time != NULL);

    elab_rtc_t *rtc = (elab_rtc_t *)me;

    elab_err_t ret = rtc->ops->set_time(rtc, rtc_time);
    if (ret != ELAB_OK)
    {
        elog_error("RTC set time error. Error ID: %d.", (int32_t)ret);
    }
}

/* ----------------------------- end of file -------------------------------- */
