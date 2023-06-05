
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if defined(__linux__)
#include <termios.h>
#include <unistd.h>
#endif
#if defined(__WIN32)
#include <conio.h>
#endif
#include "esh.h"
#include "elab_common.h"
#include "elab_assert.h"

ELAB_TAG("esh");

/* private config ----------------------------------------------------------- */
#define ESH_KEY_REG_TABLE           (32)

/* private define ----------------------------------------------------------- */
#define KEY_ENTER                   (0x0a)          /* Value of the Enter key */
#ifdef __linux__
#define KEY_ESC                     (0x03)
#else
#define KEY_ESC                     (0x1B)          /* Value of the ESC key */
#endif
#define KEY_CTRL_C                  (0x03)          /* Value of the CTRL+C key */
#define KEY_Fn                      (0x00)          /* The 1st byte of F1 - F12 */
#ifdef __linux__
#define KEY_FUNCTION                (0x1B)
#else
#define KEY_FUNCTION                (0xE0)
#endif
#define KEY_BACKSPACE               (0x08)          /* Value of the Backspace key */
#define KEY_TABLE                   (0x09)          /* Value of the Table key */

/* private typedef ---------------------------------------------------------- */
typedef struct esh_key_register
{
    uint8_t key_id;
    esh_func_t func;
    void *para;
} esh_key_register_t;

typedef struct esh
{
    esh_key_parser_t func_parser;
    esh_key_register_t key_reg_table[Esh_Max - Esh_Null];
    bool init_end;
} esh_t;

#if defined(__linux__)
#include "cmsis_os.h"

static int getch(void);
static void _entry_getch(void *parameter);

/**
 * @brief  The thread attribute for testing.
 */
static const osThreadAttr_t thread_attr_getch = 
{
    .name = "ThreadGetChar",
    .attr_bits = osThreadDetached,
    .priority = osPriorityNormal,
    .stack_size = 2048,
};

static osMessageQueueId_t mq_getch = NULL;
#endif

/* private variables -------------------------------------------------------- */
static esh_t esh =
{
    .init_end = false,
};

/* public function ---------------------------------------------------------- */
/**
  * @brief  Esh F-key (esh_key_id) parser function type.
  * @param  func_parser the F-key parser function.
  * @retval None
  */
void esh_init(esh_key_parser_t func_parser)
{
    esh.func_parser = func_parser;

    for (uint32_t i = 0; i < ESH_KEY_REG_TABLE; i ++)
    {
        esh.key_reg_table[i].key_id = Esh_Null;
        esh.key_reg_table[i].func = NULL;
        esh.key_reg_table[i].para = NULL;
    }

#if defined(__linux__)
    /*  On Linux, one specific thread is created for the function getch which 
        can check the data is timeout or not. */
    osThreadId_t _thread = osThreadNew(_entry_getch, NULL, &thread_attr_getch);
    assert(_thread != NULL);
#endif

    esh.init_end = true;
}

/**
  * @brief  Esh F-key function registering.
  * @param  key     f-key ID.
  * @param  func    Ther F-key function.
  * @param  paras   Parameters.
  * @retval None
  */
void esh_key_register(uint8_t key, esh_func_t func, void *paras)
{
    uint32_t i = key - Esh_Null;

    esh.key_reg_table[i].key_id = key;
    esh.key_reg_table[i].func = func;
    esh.key_reg_table[i].para = paras;
}

/**
  * @brief  Esh getch function.
  * @param  None
  * @retval key id.
  */
char esh_getch(void)
{
    uint8_t key_id;
#if defined(__linux__)
    char buffer[16];
    uint32_t count_ch = 0;
#endif

    /* If esh is not initialed, prevent the code enter assert. */
    if (!esh.init_end)
    {
        osDelay(100);
        key_id = Esh_Null;
        goto exit;
    }

    while (esh.init_end)
    {
        key_id = getch();
#ifdef __linux__
        if (key_id == (uint8_t)osErrorTimeout)
        {
            if (count_ch == 0)
            {
                continue;
            }
        }
        if (key_id > 0)
        {
            buffer[count_ch ++] = key_id;
            // continue;
        }
#endif

        key_id = buffer[0];

        if (key_id == KEY_ENTER)
        {
            key_id = '\n';
            break;
        }
        /* Ctrl + A -> Z. */
        else if (key_id >= 0x01 && key_id <= 0x1A &&
                    key_id != KEY_CTRL_C && key_id != KEY_TABLE &&
                    KEY_ESC != key_id)
        {
            /* Ignore the key Ctrl + A -> Z. */
        }
        /*
        else if ((KEY_FUNCTION == key_id) || (KEY_Fn == key_id))
        {
#ifndef __linux__
            buffer[1] = getch();
            count_ch = 2;
#endif
            uint8_t key_func = esh.func_parser(buffer, count_ch);
            uint8_t index = key_func - Esh_Null;
            void *para = esh.key_reg_table[index].para;
            if (esh.key_reg_table[index].func != NULL)
            {
                esh.key_reg_table[index].func(key_func, para);
            }

            count_ch = 0;
            memset(buffer, 0, 16);

            continue;
        }
        */
        else
        {
            break;
        }
    }

    count_ch = 0;
    memset(buffer, 0, 16);

exit:
    return key_id;
}

/* private function --------------------------------------------------------- */
#if defined(__linux__)
/**
  * @brief  The original __getch function for Linux which can get the input char
  *         in the terminal.
  * @retval Key id.
  */
static int __getch(void)
{
    int ch;

    struct termios tm, tm_old;
    tcgetattr(STDIN_FILENO, &tm);
    tm_old = tm;
    tm.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &tm);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &tm_old);

    return ch;
}

/**
  * @brief  The thread entry function for getch function. The thread is for 
  *         checking the char is timeout or not. 
  * @retval None.
  */
static void _entry_getch(void *parameter)
{
    mq_getch = osMessageQueueNew(16, 1, NULL);
    assert(mq_getch != NULL);

    while (1)
    {
        char ch = __getch();
        osStatus_t ret_os = osMessageQueuePut(mq_getch, &ch, 0, 0);
        assert(ret_os == osOK);
    }
}

/**
  * @brief  getch function for Linux.
  * @retval Key id if > 0, and error id if < 0.
  */
static int getch(void)
{
    char ch;
    int ret = 0;
    osStatus_t ret_os = osMessageQueueGet(mq_getch, &ch, NULL, 20);
    if (ret_os == osOK)
    {
        ret = ch;
    }
    else
    {
        ret = (int)ret_os;
    }

    return ret;
}
#endif

/* ----------------------------- end of file -------------------------------- */
