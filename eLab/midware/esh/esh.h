/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ESH_H
#define ESH_H

/* include ------------------------------------------------------------------ */
#include <stdbool.h>
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public define ------------------------------------------------------------ */
/**
  * @brief  The special key ID defines.
  */
enum esh_key_id
{
    Esh_Null = 128,
    Esh_F1, Esh_F2, Esh_F3, Esh_F4, Esh_F5, Esh_F6, 
    Esh_F7, Esh_F8, Esh_F9, Esh_F10, Esh_F12,
    Esh_Up, Esh_Down, Esh_Right, Esh_Left,
    Esh_Home, Esh_Insert, Esh_Delect, Esh_End, Esh_PageUp, Esh_PageDown,
    Esh_Max
};

/* public typedef ----------------------------------------------------------- */
/**
  * @brief  Key function type.
  */
typedef void (* esh_func_t)(uint8_t key_id, void *paras);

/**
  * @brief  Esh F-key (esh_key_id) parser function type.
  */
typedef uint8_t (* esh_key_parser_t)(uint8_t *value, uint8_t count);

/* private function --------------------------------------------------------- */
/**
  * @brief  Esh F-key (esh_key_id) parser function type.
  * @param  func_parser the F-key parser function.
  * @retval None
  */
void esh_init(esh_key_parser_t func_parser);

/**
  * @brief  Esh F-key function registering.
  * @param  key     f-key ID.
  * @param  func    Ther F-key function.
  * @param  paras   Parameters.
  * @retval None
  */
void esh_key_register(uint8_t key, esh_func_t func, void *paras);

/**
  * @brief  Esh getch function.
  * @param  None
  * @retval key id.
  */
char esh_getch(void);

#ifdef __cplusplus
}
#endif

#endif

/* ----------------------------- end of file -------------------------------- */
