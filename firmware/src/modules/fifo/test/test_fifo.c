/**
 * @file   test_fifo.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the FIFO module.
 */

/*
This file is part of CANDrive firmware.

CANDrive firmware is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CANDrive firmware is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with CANDrive firmware.  If not, see <http://www.gnu.org/licenses/>.
*/

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdbool.h>

#include "fifo.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static void FillBuffer(struct fifo_t *fifo_p, size_t number_of_elements)
{
    for (size_t cnt = 0; cnt < number_of_elements; ++cnt)
    {
        FIFO_Push(fifo_p, &cnt);
    }
    return;
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

void test_FIFO_New(void **state)
{
    uint8_t buffer[8];

    struct fifo_t fifo = FIFO_New(buffer);

    assert_ptr_equal(fifo.data_p, buffer);
    assert_int_equal(fifo.head, 0);
    assert_int_equal(fifo.tail, 0);
    assert_int_equal(fifo.element_size, 1);
    assert_int_equal(fifo.max_number_of_elements, 8);
    assert_int_equal(fifo.number_of_elements, 0);
}

void test_FIFO_Push_NULL_arguments(void **state)
{
    uint8_t dummy_item;
    struct fifo_t dummy_fifo;

    expect_assert_failure(FIFO_Push(NULL, &dummy_item));
    expect_assert_failure(FIFO_Push(&dummy_fifo, NULL));
    expect_assert_failure(FIFO_Push(NULL, NULL));
}


void test_FIFO_Push_Empty(void **state)
{
    uint8_t buffer[8] = {0};
    uint8_t item = 0xAA;
    struct fifo_t fifo = FIFO_New(buffer);

    // We expect true when pushing to a empty FIFO.
    assert_true(FIFO_Push(&fifo, &item));

    // Make sure the FIFO content is correct.
    uint8_t expected_buffer_content[8] = {0xAA, 0, 0, 0, 0, 0, 0, 0};
    assert_memory_equal(expected_buffer_content, buffer,
                        sizeof(expected_buffer_content));
}

void test_FIFO_Push_Full(void **state)
{
    uint8_t buffer[8] = {0};
    uint8_t item = 0xAA;
    struct fifo_t fifo = FIFO_New(buffer);

    // Fill the FIFO with numbers 0 to 7.
    FillBuffer(&fifo, sizeof(buffer));

    // We expect false when pushing to a full FIFO.
    assert_false(FIFO_Push(&fifo, &item));

    // Make sure the FIFO content is unchanged.
    uint8_t expected_buffer_content[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    assert_memory_equal(expected_buffer_content, buffer,
                        sizeof(expected_buffer_content));
}

void test_FIFO_Pop_NULL_arguments(void **state)
{
    uint8_t dummy_item;
    struct fifo_t dummy_fifo;

    expect_assert_failure(FIFO_Pop(NULL, &dummy_item));
    expect_assert_failure(FIFO_Pop(&dummy_fifo, NULL));
    expect_assert_failure(FIFO_Pop(NULL, NULL));
}

void test_FIFO_Pop_Empty(void **state)
{
    uint8_t buffer[8];
    uint8_t item;
    struct fifo_t fifo = FIFO_New(buffer);

    // We expect false when poping from a empty FIFO.
    assert_false(FIFO_Pop(&fifo, &item));
}

void test_FIFO_Pop_NonEmpty(void **state)
{
    uint8_t buffer[8];
    uint8_t item = 0xAA;

    struct fifo_t fifo = FIFO_New(buffer);
    FIFO_Push(&fifo, &item);

    // Clear item so we know for sure that it is set by FIFO_Pop().
    item = 0x00;

    // We expect true when popping from a non empty FIFO.
    assert_true(FIFO_Pop(&fifo, &item));

    // Check if we got the correct value.
    assert_int_equal(0xAA, item);

    // Since we only pushed one item to the FIFO it should be empty here.
    assert_true(FIFO_IsEmpty(&fifo));
}

void test_FIFO_Peek_NULL_arguments(void **state)
{
    uint8_t dummy_item;
    struct fifo_t dummy_fifo;

    expect_assert_failure(FIFO_Peek(NULL, &dummy_item));
    expect_assert_failure(FIFO_Peek(&dummy_fifo, NULL));
    expect_assert_failure(FIFO_Peek(NULL, NULL));
}

void test_FIFO_Peek_Empty(void **state)
{
    uint8_t buffer[8];
    uint8_t item;
    struct fifo_t fifo = FIFO_New(buffer);

    // We expect false when peeking from a empty FIFO.
    assert_false(FIFO_Peek(&fifo, &item));
}

void test_FIFO_Peek_NonEmpty(void **state)
{
    uint8_t buffer[8];
    uint8_t item = 0xAA;

    struct fifo_t fifo = FIFO_New(buffer);
    FIFO_Push(&fifo, &item);

    // Clear item so we know for sure that it is set by FIFO_Peek().
    item = 0x00;

    // We expect true when peeking from a non empty FIFO.
    assert_true(FIFO_Peek(&fifo, &item));

    // Check if we got the correct value.
    assert_int_equal(0xAA, item);

    // The FIFO should be unchanged by the peek.
    assert_false(FIFO_IsEmpty(&fifo));
}

void test_FIFO_IsEmpty_NULL(void **state)
{
    expect_assert_failure(FIFO_IsEmpty(NULL));
}

void test_FIFO_IsEmpty_Empty(void **state)
{
    uint8_t buffer[8];
    struct fifo_t fifo = FIFO_New(buffer);

    assert_true(FIFO_IsEmpty(&fifo));
}

void test_FIFO_IsEmpty_Full(void **state)
{
    uint8_t buffer[8];
    struct fifo_t fifo = FIFO_New(buffer);
    FillBuffer(&fifo, sizeof(buffer));

    assert_false(FIFO_IsEmpty(&fifo));
}

void test_FIFO_IsEmpty_NotEmpty(void **state)
{
    uint8_t buffer[8];
    struct fifo_t fifo = FIFO_New(buffer);
    FillBuffer(&fifo, sizeof(buffer) / 2);

    assert_false(FIFO_IsEmpty(&fifo));
}

void test_FIFO_IsFull_NULL(void **state)
{
    expect_assert_failure(FIFO_IsFull(NULL));
}

void test_FIFO_IsFull_Empty(void **state)
{
    uint8_t buffer[8];
    struct fifo_t fifo = FIFO_New(buffer);

    assert_false(FIFO_IsFull(&fifo));
}

void test_FIFO_IsFull_Full(void **state)
{
    uint8_t buffer[8];
    struct fifo_t fifo = FIFO_New(buffer);
    FillBuffer(&fifo, sizeof(buffer));

    assert_true(FIFO_IsFull(&fifo));
}

void test_FIFO_IsFull_NotEmpty(void **state)
{
    uint8_t buffer[8];
    struct fifo_t fifo = FIFO_New(buffer);
    FillBuffer(&fifo, sizeof(buffer) / 2);

    assert_false(FIFO_IsFull(&fifo));
}

void test_FIFO_Clear_NULL(void **state)
{
    expect_assert_failure(FIFO_Clear(NULL));
}

void test_FIFO_Clear_Full(void **state)
{
    uint8_t buffer[8];
    struct fifo_t fifo = FIFO_New(buffer);
    FillBuffer(&fifo, sizeof(buffer));

    FIFO_Clear(&fifo);
    assert_true(FIFO_IsEmpty(&fifo));
}

void test_FIFO_GetAvailableSlots_NULL(void **state)
{
    expect_assert_failure(FIFO_GetAvailableSlots(NULL));
}

void test_FIFO_GetAvailableSlots_Empty(void **state)
{
    uint8_t buffer[8];
    struct fifo_t fifo = FIFO_New(buffer);

    assert_int_equal(FIFO_GetAvailableSlots(&fifo), sizeof(buffer));
}

void test_FIFO_GetAvailableSlots_Full(void **state)
{
    uint8_t buffer[8];
    struct fifo_t fifo = FIFO_New(buffer);
    FillBuffer(&fifo, sizeof(buffer));

    assert_int_equal(FIFO_GetAvailableSlots(&fifo), 0);
}

void test_FIFO_GetAvailableSlots_NotEmpty(void **state)
{
    uint8_t buffer[8];
    struct fifo_t fifo = FIFO_New(buffer);
    FillBuffer(&fifo, sizeof(buffer) / 2);

    assert_int_equal(FIFO_GetAvailableSlots(&fifo), sizeof(buffer) / 2);
}

void test_FIFO_MaxSize(void **state)
{
    struct dummy_item
    {
        uint8_t data[UINT8_MAX];
    };
    struct dummy_item buffer[UINT8_MAX];
    struct fifo_t fifo = FIFO_New(buffer);

    for (size_t i = 0; i < UINT8_MAX; ++i)
    {
        struct dummy_item item = {i};
        assert_true(FIFO_Push(&fifo, &item));
    }
    assert_true(FIFO_IsFull(&fifo));

    for (size_t i = 0; i < UINT8_MAX; ++i)
    {
        struct dummy_item item;
        assert_true(FIFO_Pop(&fifo, &item));
        assert_int_equal(i, item.data[0]);
    }
    assert_true(FIFO_IsEmpty(&fifo));
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_FIFO[] =
    {
        cmocka_unit_test(test_FIFO_New),
        cmocka_unit_test(test_FIFO_Push_NULL_arguments),
        cmocka_unit_test(test_FIFO_Push_Empty),
        cmocka_unit_test(test_FIFO_Push_Full),
        cmocka_unit_test(test_FIFO_Pop_NULL_arguments),
        cmocka_unit_test(test_FIFO_Pop_Empty),
        cmocka_unit_test(test_FIFO_Pop_NonEmpty),
        cmocka_unit_test(test_FIFO_Peek_NULL_arguments),
        cmocka_unit_test(test_FIFO_Peek_Empty),
        cmocka_unit_test(test_FIFO_Peek_NonEmpty),
        cmocka_unit_test(test_FIFO_IsEmpty_NULL),
        cmocka_unit_test(test_FIFO_IsEmpty_Empty),
        cmocka_unit_test(test_FIFO_IsEmpty_Full),
        cmocka_unit_test(test_FIFO_IsEmpty_NotEmpty),
        cmocka_unit_test(test_FIFO_IsFull_NULL),
        cmocka_unit_test(test_FIFO_IsFull_Empty),
        cmocka_unit_test(test_FIFO_IsFull_Full),
        cmocka_unit_test(test_FIFO_IsFull_NotEmpty),
        cmocka_unit_test(test_FIFO_Clear_NULL),
        cmocka_unit_test(test_FIFO_Clear_Full),
        cmocka_unit_test(test_FIFO_GetAvailableSlots_NULL),
        cmocka_unit_test(test_FIFO_GetAvailableSlots_Empty),
        cmocka_unit_test(test_FIFO_GetAvailableSlots_Full),
        cmocka_unit_test(test_FIFO_GetAvailableSlots_NotEmpty),
        cmocka_unit_test(test_FIFO_MaxSize),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_FIFO, NULL, NULL);
}
