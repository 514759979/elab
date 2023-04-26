
#include <stdlib.h>
#include <stdio.h>
#if defined(__linux__)
#include <termios.h>
#include <unistd.h>
#endif
#if defined(__WIN32)
#include <conio.h>
#endif
#include "esh.h"


#define EXSH_KEY_REG_TABLE          (32)

#define KEY_ENTER                   (0x0a)          // Value of the Enter key
#define KEY_ESC                     (0x1B)          // Value of the ESC key
#define KEY_CTRL_C                  (0x03)          // Value of the CTRL+C key
#define KEY_Fn                      (0x00)          // The 1st byte of F1 - F12
#define KEY_FUNCTION                (0xE0)
#define KEY_BACKSPACE               (0x08)          // Value of the Backspace key

typedef struct exsh_key_register
{
    uint8_t key_id;
    exsh_func_t func;
    void *para;
} exsh_key_register_t;

typedef struct exsh
{
    exsh_key_parser_t func_parser;
    exsh_func_t func_normal;
    exsh_func_exit_t func_exit;
    void *nomarl_para;

    exsh_key_register_t key_reg_table[Esh_Max - Esh_Null];
} exsh_t;

static exsh_t exsh;

void ex_shell_init(exsh_key_parser_t func_parser,
                   exsh_func_t func,
                   exsh_func_exit_t func_exit,
                   void *paras)
{
    exsh.func_normal = func;
    exsh.nomarl_para = paras;
    exsh.func_parser = func_parser;
    exsh.func_exit = func_exit;

    for (uint32_t i = 0; i < EXSH_KEY_REG_TABLE; i ++)
    {
        exsh.key_reg_table[i].key_id = Esh_Null;
        exsh.key_reg_table[i].func = NULL;
        exsh.key_reg_table[i].para = NULL;
    }
}

void ex_shell_key_register(uint8_t key, exsh_func_t func, void *paras)
{
    uint32_t i = key - Esh_Null;

    exsh.key_reg_table[i].key_id = key;
    exsh.key_reg_table[i].func = func;
    exsh.key_reg_table[i].para = paras;
}

#if defined(__linux__)
static int getch(void)
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
#endif

char esh_getch(void)
{
    char key_id;

    while (1)
    {
        key_id = getch();
        if ((KEY_ESC == key_id))
        {
            exsh.func_exit(exsh.nomarl_para);
        }
        else if (key_id == KEY_ENTER)
        {
            key_id = '\n';
            break;
        }
        /* Ctrl + A -> Z. */
        else if (key_id >= 0x01 && key_id <= 0x1A && key_id != KEY_CTRL_C)
        {
            /* Ignore the key Ctrl + A -> Z. */
        }
        else if ((KEY_FUNCTION == key_id) || (KEY_Fn == key_id))
        {
            uint8_t key_func_origin[2] =
            {
                key_id, getch()
            };
            uint8_t key_func = exsh.func_parser(key_func_origin, 2);
            uint8_t index = key_func - Esh_Null;
            void *para = exsh.key_reg_table[index].para;
            if (exsh.key_reg_table[index].func != NULL)
            {
                exsh.key_reg_table[index].func(key_func, para);
            }
        }
        else
        {
            break;
        }
    }

    return key_id;
}
