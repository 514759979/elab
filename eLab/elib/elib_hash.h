/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ELIB_HASH_H
#define ELIB_HASH_H

/* includes ----------------------------------------------------------------- */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* public functions --------------------------------------------------------- */
uint32_t elib_hash_time33(char *str);
uint32_t elib_hash_sdbm(char *str);
uint32_t elib_hash_rs(char *str);
uint32_t elib_hash_js(char *str);
uint32_t elib_hash_djb(char *str);
uint32_t elib_hash_bkdr(char *str);
uint32_t elib_hash_elf(char *str);
uint32_t elib_hash_pjw(char *str);
uint32_t elib_hash_ap(char *str);

#ifdef __cplusplus
}
#endif

#endif

/* ----------------------------- end of file -------------------------------- */
