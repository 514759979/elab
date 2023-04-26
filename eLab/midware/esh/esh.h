#ifndef ESH_H
#define ESH_H

#include <stdbool.h>
#include "stdint.h"

enum esh_key_id
{
    Esh_Null = 128,
    Esh_F1, Esh_F2, Esh_F3, Esh_F4, Esh_F5, Esh_F6, 
    Esh_F7, Esh_F8, Esh_F9, Esh_F10, Esh_F12,
    Esh_Up, Esh_Down, Esh_Right, Esh_Left,
    Esh_Home, Esh_Insert, Esh_Delect, Esh_End, Esh_PageUp, Esh_PageDown,
    Esh_Max
};

typedef void (* exsh_func_exit_t)(void *paras);
typedef void (* exsh_func_t)(uint8_t key_id, void *paras);
typedef uint8_t (* exsh_key_parser_t)(uint8_t *key, uint8_t count);

void esh_init(exsh_key_parser_t func_parser,
                exsh_func_t func,
                exsh_func_exit_t func_exit,
                void *paras);
void esh_key_register(uint8_t key, exsh_func_t func, void *paras);
char esh_getch(void);

#endif
