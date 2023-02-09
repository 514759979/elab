/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@oelib_queuelook.com>
 */

/* include ------------------------------------------------------------------ */
#include <string.h>
#include "elab_def.h"
#include "elab_export.h"
#include "elib_queue.h"
#include "unity.h"
#include "unity_fixture.h"

/* private config ----------------------------------------------------------- */
#define ELIB_QUEUE_CAPACITY                     (64)

/* private variables -------------------------------------------------------- */
static elib_queue_t queue;
static uint8_t buffer[ELIB_QUEUE_CAPACITY];
static uint8_t buffer_export[ELIB_QUEUE_CAPACITY];

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  The elib_queue test group.
 */
TEST_GROUP(elib_queue);

/**
 * @brief  The setup function of elib_queue test group.
 */
TEST_SETUP(elib_queue)
{
    memset(buffer, 0, ELIB_QUEUE_CAPACITY);
    elib_queue_init(&queue, buffer, ELIB_QUEUE_CAPACITY);
    TEST_ASSERT_FALSE(elib_queue_is_full(&queue));
    TEST_ASSERT_TRUE(elib_queue_is_empty(&queue));
    TEST_ASSERT_EQUAL_UINT16(ELIB_QUEUE_CAPACITY, elib_queue_free_size(&queue));
}

/**
 * @brief  The teardown function of elib_queue test group.
 */
TEST_TEAR_DOWN(elib_queue)
{
}

/**
 * @brief  The unit test only for elib_queue_push.
 */
TEST(elib_queue, push)
{
    int32_t ret = 1;
    uint8_t byte = 0;

    /* Check the queue is not full and not empty. */
    for (uint16_t i = 0; i < (ELIB_QUEUE_CAPACITY - 1); i ++)
    {
        TEST_ASSERT_EQUAL_UINT16((ELIB_QUEUE_CAPACITY - i),
                                    elib_queue_free_size(&queue));
        byte = i + 1;
        ret = elib_queue_push(&queue, &byte, 1);
        TEST_ASSERT_EQUAL_INT32(1, ret);
        TEST_ASSERT_FALSE(elib_queue_is_full(&queue));
        TEST_ASSERT_FALSE(elib_queue_is_empty(&queue));
        TEST_ASSERT_EQUAL_UINT16((ELIB_QUEUE_CAPACITY - i - 1),
                                    elib_queue_free_size(&queue));
    }

    /* Check the queue is full */
    TEST_ASSERT_EQUAL_UINT16(1, elib_queue_free_size(&queue));
    byte = ELIB_QUEUE_CAPACITY;
    ret = elib_queue_push(&queue, &byte, 1);
    TEST_ASSERT_EQUAL_INT32(1, ret);
    TEST_ASSERT_TRUE(elib_queue_is_full(&queue));
    TEST_ASSERT_FALSE(elib_queue_is_empty(&queue));
    TEST_ASSERT_EQUAL_UINT16(0, elib_queue_free_size(&queue));

    /* Check the queue's data. */
    for (uint16_t i = 0; i < ELIB_QUEUE_CAPACITY; i ++)
    {
        buffer_export[i] = (uint8_t)(i + 1);
    }
    TEST_ASSERT_EQUAL_UINT8_ARRAY(buffer_export, buffer, ELIB_QUEUE_CAPACITY);

    /* Check the queue is not enough. */
    ret = elib_queue_push(&queue, &byte, 1);
    TEST_ASSERT_EQUAL_INT32((int32_t)ELAB_ERR_NOT_ENOUGH, ret);
}

/**
 * @brief  The unit test only for elib_queue_pull and elib_queue_pop.
 */
TEST(elib_queue, pull_then_pop)
{
    int32_t ret = 1;
    uint8_t byte = 0;

    for (uint16_t i = 0; i < ELIB_QUEUE_CAPACITY; i ++)
    {
        byte = i + 1;
        ret = elib_queue_push(&queue, &byte, 1);
        TEST_ASSERT_EQUAL_INT32(1, ret);
    }

    for (uint16_t i = 0; i < ELIB_QUEUE_CAPACITY; i ++)
    {
        ret = elib_queue_pull(&queue, &byte, 1);
        TEST_ASSERT_EQUAL_INT32(1, ret);
        TEST_ASSERT_EQUAL_UINT8((i + 1), byte);
        TEST_ASSERT_EQUAL_UINT16(i, elib_queue_free_size(&queue));

        ret = elib_queue_pop(&queue, 1);
        TEST_ASSERT_EQUAL_INT32(1, ret);
        TEST_ASSERT_EQUAL_UINT16((i + 1), elib_queue_free_size(&queue));

        TEST_ASSERT_FALSE(elib_queue_is_full(&queue));
        if (i < (ELIB_QUEUE_CAPACITY - 1))
        {
            TEST_ASSERT_FALSE(elib_queue_is_empty(&queue));
        }
        else
        {
            TEST_ASSERT_TRUE(elib_queue_is_empty(&queue));
        }
    }
}

/**
 * @brief  The unit test only for elib_queue_pull_pop.
 */
TEST(elib_queue, pull_and_pop)
{
    int32_t ret = 1;
    uint8_t byte = 0;

    for (uint16_t i = 0; i < ELIB_QUEUE_CAPACITY; i ++)
    {
        byte = i + 1;
        ret = elib_queue_push(&queue, &byte, 1);
        TEST_ASSERT_EQUAL_INT32(1, ret);
    }

    for (uint16_t i = 0; i < ELIB_QUEUE_CAPACITY; i ++)
    {
        TEST_ASSERT_EQUAL_UINT16(i, elib_queue_free_size(&queue));
        ret = elib_queue_pull_pop(&queue, &byte, 1);
        TEST_ASSERT_EQUAL_INT32(1, ret);
        TEST_ASSERT_EQUAL_UINT16((i + 1), elib_queue_free_size(&queue));

        if (i < (ELIB_QUEUE_CAPACITY - 1))
        {
            TEST_ASSERT_FALSE(elib_queue_is_empty(&queue));
        }
        else
        {
            TEST_ASSERT_TRUE(elib_queue_is_empty(&queue));
        }
    }
}

/**
 * @brief  The unit test only for elib_queue_clear.
 */
TEST(elib_queue, clear)
{
    int32_t ret = 1;
    uint8_t byte = 0;

    for (uint16_t i = 0; i < ELIB_QUEUE_CAPACITY; i ++)
    {
        byte = i + 1;
        ret = elib_queue_push(&queue, &byte, 1);
        TEST_ASSERT_EQUAL_INT32(1, ret);
    }

    elib_queue_clear(&queue);
    TEST_ASSERT_FALSE(elib_queue_is_full(&queue));
    TEST_ASSERT_TRUE(elib_queue_is_empty(&queue));
    TEST_ASSERT_EQUAL_UINT16(ELIB_QUEUE_CAPACITY, elib_queue_free_size(&queue));
}

/**
 * @brief  The elib_queue group unit test.
 */
TEST_GROUP_RUNNER(elib_queue)
{
    RUN_TEST_CASE(elib_queue, push);
    RUN_TEST_CASE(elib_queue, pull_then_pop);
    RUN_TEST_CASE(elib_queue, pull_and_pop);
    RUN_TEST_CASE(elib_queue, clear);
}

/* ----------------------------- end of file -------------------------------- */
