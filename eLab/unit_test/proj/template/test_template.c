/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include "elab_export.h"
#include "unity.h"
#include "unity_fixture.h"

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  The ut test group.
 */
TEST_GROUP(ut);

/**
 * @brief  The setup function of ut test group.
 */
TEST_SETUP(ut)
{
}

/**
 * @brief  The teardown function of ut test group.
 */
TEST_TEAR_DOWN(ut)
{
}

/**
 * @brief  The unit test only for test_01.
 */
TEST(ut, test_01)
{
}

/**
 * @brief  The unit test only for test_02.
 */
TEST(ut, test_02)
{
}

/**
 * @brief  The ut group unit test.
 */
TEST_GROUP_RUNNER(ut)
{
    RUN_TEST_CASE(ut, test_01);
    RUN_TEST_CASE(ut, test_02);
}

/* ----------------------------- end of file -------------------------------- */
