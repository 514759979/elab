/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include <string.h>
#include "edb.h"
#include "../../common/elab_assert.h"
#include "../../3rd/ini/ini.h"
#include "../../RTOS/cmsis_os.h"

ELAB_TAG("edb");

/* private config ----------------------------------------------------------- */
#define EDB_BUFFER_SIZE                         (256)

/* private function prototypes ---------------------------------------------- */
static int32_t _get_string_index(char *buff_line, uint8_t index,
                                    char *str, uint32_t size);
static uint32_t _get_string(const char *section,
                            const char *key,
                            char *str,
                            uint32_t size);

/* private variables -------------------------------------------------------- */
static ini_t *edb_ini = NULL;
static osMutexId_t *mutex_edb = NULL;
static char buff_line[EDB_BUFFER_SIZE];

/**
 * The edb global mutex attribute.
 */
static const osMutexAttr_t mutex_attr_edb =
{
    "mutex_edb", osMutexRecursive | osMutexPrioInherit, NULL, 0U 
};

/* public function ---------------------------------------------------------- */
void edb_init(const char *path)
{
    edb_ini = ini_load(path);
    elab_assert(edb_ini != NULL);

    mutex_edb = osMutexNew(&mutex_attr_edb);
    elab_assert(mutex_edb != NULL);
}

uint32_t edb_get_hex32(const char *section, const char *key)
{
    uint32_t value_u32 = 0;
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    int ret_ini = 0;
    ret_ini = ini_sget(edb_ini, section, key, "%x", &value_u32);
    elab_assert(ret_ini == 1);

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return value_u32;
}

uint32_t edb_get_u32(const char *section, const char *key)
{
    uint32_t value_u32 = 0;
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    int ret_ini = 0;
    ret_ini = ini_sget(edb_ini, section, key, "%u", &value_u32);
    elab_assert(ret_ini == 1);

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return value_u32;
}

int32_t edb_get_s32(const char *section, const char *key)
{
    int32_t value_s32 = 0;
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    int ret_ini = 0;
    ret_ini = ini_sget(edb_ini, section, key, "%d", &value_s32);
    elab_assert(ret_ini == 1);

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return value_s32;
}

uint64_t edb_get_u64(const char *section, const char *key)
{
    uint32_t value_u64 = 0;
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    int ret_ini = 0;
    ret_ini = ini_sget(edb_ini, section, key, "%lu", &value_u64);
    elab_assert(ret_ini == 1);

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return value_u64;
}

int64_t edb_get_s64(const char *section, const char *key)
{
    uint32_t value_s64 = 0;
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    int ret_ini = 0;
    ret_ini = ini_sget(edb_ini, section, key, "%ld", &value_s64);
    elab_assert(ret_ini == 1);

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return value_s64;
}

bool edb_get_bool(const char *section, const char *key)
{
    bool value = false;
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    const char *str = ini_get(edb_ini, section, key);
    elab_assert(str != NULL);

    if (strcmp(str, "true") == 0)
    {
        value = true;
    }
    else if (strcmp(str, "true") == 0)
    {
        value = false;
    }
    else
    {
        elab_assert(false);
    }

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return value;
}

float edb_get_float(const char *section, const char *key)
{
    float value = 0;
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    int ret_ini = 0;
    ret_ini = ini_sget(edb_ini, section, key, "%f", &value);
    elab_assert(ret_ini == 1);

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return value;
}

double edb_get_double(const char *section, const char *key)
{
    double value = 0;
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    int ret_ini = 0;
    ret_ini = ini_sget(edb_ini, section, key, "%lf", &value);
    elab_assert(ret_ini == 1);

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return value;
}

uint32_t edb_get_string(const char *section,
                        const char *key,
                        char *str,
                        uint32_t size)
{
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    uint32_t size_cp = _get_string(section, key, str, size);

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return size_cp;
}

bool edb_str_cmp(const char *section, const char *key, const char *str)
{
    bool ret = false;

    osStatus_t ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    uint32_t size_cp = _get_string(section, key, buff_line, (EDB_BUFFER_SIZE - 1));
    if (strcmp(buff_line, str) == 0)
    {
        ret = true;
    }

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return ret;
}

uint32_t edb_get_sub_u32(const char *section, const char *key, uint8_t index)
{
    uint32_t value = 0;

    #define EDB_BUFF_SIZE_U32                   (12)

    char buff[EDB_BUFF_SIZE_U32];
    memset(buff, 0, EDB_BUFF_SIZE_U32);
    edb_get_sub_string(section, key, index, buff, EDB_BUFF_SIZE_U32);
    value = atoi(buff);

    #undef EDB_BUFF_SIZE_U32

    return value;
}

float edb_get_sub_float(const char *section, const char *key, uint8_t index)
{
    float value = 0;

    #define EDB_BUFF_SIZE_FLOAT                   (12)

    char buff[EDB_BUFF_SIZE_FLOAT];
    memset(buff, 0, EDB_BUFF_SIZE_FLOAT);
    edb_get_sub_string(section, key, index, buff, EDB_BUFF_SIZE_FLOAT);
    value = atof(buff);

    #undef EDB_BUFF_SIZE_U32

    return value;
}

uint32_t edb_get_sub_string(const char *section,
                            const char *key,
                            uint8_t index,
                            char *str, uint32_t size)
{
    uint32_t ret_size = 0;
    osStatus_t ret_os = osOK;

    ret_os = osMutexAcquire(mutex_edb, osWaitForever);
    elab_assert(ret_os == osOK);

    uint32_t size_cp = _get_string(section, key, buff_line, (EDB_BUFFER_SIZE - 1));
    ret_size = _get_string_index(buff_line, index, str, (size - 1));

    ret_os = osMutexRelease(mutex_edb);
    elab_assert(ret_os == osOK);

    return ret_size;
}

/* private function --------------------------------------------------------- */
static uint32_t _get_string(const char *section,
                            const char *key,
                            char *str,
                            uint32_t size)
{
    const char *_str = ini_get(edb_ini, section, key);
    elab_assert(str != NULL);

    uint32_t size_cp = strlen(_str);
    size_cp = (size < size_cp) ? size : size_cp;
    memcpy(str, _str, size_cp);
    str[size_cp] = 0;

    return size_cp;
}

static int32_t _get_string_index(char *buff_line, uint8_t index,
                                    char *str, uint32_t size)
{
    int32_t ret = 0;
    char *str_start = buff_line;
    uint32_t count_cp = 0;

    if (index > 0)
    {
        uint8_t interval_count = 0;
        bool existent = false;
        for (uint32_t i = 0; i < strlen(buff_line); i ++)
        {
            if (buff_line[i] == ',')
            {
                interval_count ++;
                if (interval_count == index)
                {
                    str_start = &buff_line[i + 1];
                    existent = true;
                    break;
                }
            }
        }

        if (!existent)
        {
            ret = -1;
            elog_error("Buffer %s has not so much intervals.", buff_line);
            goto exit;
        }
    }

    while (1)
    {
        str[count_cp] = str_start[count_cp];
        count_cp ++;
        if (count_cp > size || str_start[count_cp] == '\n' ||
            str_start[count_cp] == '\r' || str_start[count_cp] == ',')
        {
            break;
        }
    }

exit:
    return ret;
}

/* ----------------------------- end of file -------------------------------- */
