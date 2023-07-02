
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include "elab_export.h"
#include "unity.h"
#include "unity_fixture.h"

/* public function ---------------------------------------------------------- */
/**
  * @brief  The main function.
  */
int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return UnityMain(argc, (const char**)argv, elab_unit_test);
}

/* ----------------------------- end of file -------------------------------- */
