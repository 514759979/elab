/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ELAB_DEVICE_H
#define ELAB_DEVICE_H

/* include ------------------------------------------------------------------ */
#include <stdbool.h>
#include "elab_def.h"
#include "cmsis_os.h"

/* public define ------------------------------------------------------------ */
#define ELAB_DEVICE_NAME_MAX                    (32)

/* public typedef ----------------------------------------------------------- */
typedef struct elab_device
{
    const char *name;
    void *user_data;
    osMutexId_t mutex;
} elab_device_t;

/* public function ---------------------------------------------------------- */
bool elab_device_valid(const char *dev_name);
elab_device_t *elab_device_find(const char *name);
void elab_device_register(elab_device_t *const me, const char *name, void *user_data);
void elab_device_lock(elab_device_t *const me);
void elab_device_unlock(elab_device_t *const me);

#endif  /* ELAB_DEVICE_H */

/* ----------------------------- end of file -------------------------------- */
