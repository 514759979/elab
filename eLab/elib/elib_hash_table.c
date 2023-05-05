/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdlib.h>
#include "elib_hash_table.h"
#include "elib_hash.h"
#include "elab_assert.h"
#include "elab_common.h"

ELAB_TAG("eLibHashTable");

/* private function prototypes ---------------------------------------------- */
static uint32_t _get_prime_max(uint32_t capacity);

/* public functions --------------------------------------------------------- */
elib_hash_table_t *elib_hash_table_new(uint32_t capacity)
{
    elib_hash_table_t *me;
    elib_hash_table_data_t *table;

    me = elab_malloc(sizeof(elib_hash_table_t));
    elab_assert(me != NULL);

    table = elab_malloc(sizeof(elib_hash_table_data_t) * capacity);
    elab_assert(table != NULL);

    elib_hash_table_init(me, table, capacity);

    return me;
}

void elib_hash_table_destroy(elib_hash_table_t * const me)
{
    elab_free(me->table);
    me->table = NULL;
    elab_free(me);
}

void elib_hash_table_init(elib_hash_table_t * const me,
                            elib_hash_table_data_t *table,
                            uint32_t capacity)
{
    me->table = table;
    me->capacity = capacity;

    for (uint32_t i = 0; i < capacity; i ++)
    {
        me->table[i].hash_time33 = UINT32_MAX;
        me->table[i].hash_elf = UINT32_MAX;
        me->table[i].hash_bkdr = UINT32_MAX;
        me->table[i].data = NULL;
    }

    me->prime_max = _get_prime_max(capacity);
}

elab_err_t elib_hash_table_add(elib_hash_table_t * const me, char *name, void *data)
{
    elab_err_t ret = ELAB_OK;

    uint32_t hash_time33 = elib_hash_time33(name);
    uint32_t hash_elf = elib_hash_elf(name);
    uint32_t hash_bkdr = elib_hash_bkdr(name);

    uint32_t index = hash_time33 % me->prime_max;
    uint32_t times_count = 0;
    for (uint32_t i = 0; i < me->capacity; i ++)
    {
        if (me->table[index + i].data == NULL)
        {
            me->table[index + i].data = data;
            me->table[index + i].hash_time33 = hash_time33;
            me->table[index + i].hash_elf = hash_elf;
            me->table[index + i].hash_bkdr = hash_bkdr;

            break;
        }
        else
        {
            times_count ++;
            elab_assert(times_count <= ELIB_HASH_SEEK_TIMES_MAX);
        }
    }

    return ret;
}

void *elib_hash_table_get(elib_hash_table_t * const me, char *name)
{
    uint32_t _index = elib_hash_talbe_index(me, name);

    return _index == UINT32_MAX ? NULL : me->table[_index].data;
}

bool elib_hash_table_existent(elib_hash_table_t * const me, char *name)
{
    return elib_hash_talbe_index(me, name) == UINT32_MAX ? false : true;
}

uint32_t elib_hash_talbe_index(elib_hash_table_t * const me, char *name)
{
    uint32_t ret = UINT32_MAX;
    uint32_t hash_time33 = elib_hash_time33(name);
    uint32_t hash_elf = elib_hash_elf(name);
    uint32_t hash_bkdr = elib_hash_bkdr(name);

    uint32_t index = hash_time33 % me->prime_max;
    uint32_t times_count = 0;
    for (uint32_t i = 0; i < me->capacity; i ++)
    {
        uint32_t _index = (index + i) % me->capacity;
        if (me->table[_index].data == NULL)
        {
            break;
        }
        if (me->table[_index].hash_time33 == hash_time33 &&
            me->table[_index].hash_elf == hash_elf &&
            me->table[_index].hash_bkdr == hash_bkdr)
        {
            ret = _index;
            break;
        }
        else
        {
            times_count ++;
            if (times_count > ELIB_HASH_SEEK_TIMES_MAX)
            {
                break;
            }
        }
    }

    return ret;
}

/* private functions -------------------------------------------------------- */
static uint32_t _get_prime_max(uint32_t capacity)
{
    uint32_t prime_max = 0;

    for (int64_t i = capacity; i > 0; i --)
    {
        bool is_prime = true;
        for (uint32_t j = 2; j < capacity; j++)
        {
            if (i <= j)
            {
                break;
            }
            if ((i % j) == 0)
            {
                is_prime = false;
                break;
            }
        }
        if (is_prime)
        {
            prime_max = i;
            break;
        }
    }

    return prime_max;
}

/* ----------------------------- end of file -------------------------------- */
