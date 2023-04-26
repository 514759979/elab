/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include <stdint.h>
#include "elab_def.h"

elab_err_t edb_init(uint32_t capacity);
bool edb_key_exist(char *key);

void edb_set_value_u32(char *key, uint32_t value);
void edb_set_value_s32(char *key, int32_t value);
void edb_set_value_u64(char *key, uint64_t value);
void edb_set_value_s64(char *key, int64_t value);
void edb_set_value_float(char *key, float value);
void edb_set_value_double(char *key, double value);
void edb_set_value_string(char *key, char *string);
void edb_set_value_data(char *key, void *buffer, uint32_t size);

uint32_t edb_get_value_u32(char *key);
int32_t edb_get_value_s32(char *key);
uint64_t edb_get_value_u64(char *key);
int32_t edb_get_value_s64(char *key);
float edvb_get_value_float(char *key);
double edvb_get_value_double(char *key);
uint32_t edb_get_value_string(char *key, char *buffer, uint32_t size);
uint32_t edb_get_value_data(char *key, void *buffer, uint32_t size);

/* ----------------------------- end of file -------------------------------- */
