/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ELIB_HASH_TABLE_H
#define ELIB_HASH_TABLE_H

/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <stdbool.h>
#include "elab_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public typedef ----------------------------------------------------------- */
#define ELIB_HASH_SEEK_TIMES_MAX                (32)

/* public typedef ----------------------------------------------------------- */
typedef struct elib_hash_table_data
{
    uint32_t hash_time33;
    uint32_t hash_elf;
    uint32_t hash_bkdr;
    void *data;
} elib_hash_table_data_t;

typedef struct elib_hash_table
{
    uint32_t capacity;
    uint32_t prime_max;
    elib_hash_table_data_t *table;
} elib_hash_table_t;

/* public functions --------------------------------------------------------- */
elib_hash_table_t *elib_hash_table_new(uint32_t capacity);
void elib_hash_table_destroy(elib_hash_table_t * const me);
void elib_hash_table_init(elib_hash_table_t * const me,
                            elib_hash_table_data_t *table,
                            uint32_t capacity);
elab_err_t elib_hash_table_add(elib_hash_table_t * const me, char *name, void *data);
void *elib_hash_table_get(elib_hash_table_t * const me, char *name);
bool elib_hash_table_existent(elib_hash_table_t * const me, char *name);
uint32_t elib_hash_talbe_index(elib_hash_table_t * const me, char *name);

#ifdef __cplusplus
}
#endif

#endif  /* ELIB_HASH_TABLE_H */

/* ----------------------------- end of file -------------------------------- */
