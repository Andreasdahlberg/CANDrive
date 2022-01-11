/**
 * @file   test_strean.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the Stream module.
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
#include <string.h>
#include "stream.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct stream_t stream;
static uint8_t buffer[16];

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static int Setup(void **state)
{
    memset(&stream, 0, sizeof(stream));
    memset(buffer, 0, sizeof(buffer));

    Stream_Init(&stream, buffer, sizeof(buffer));
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

void test_Stream_Init_Invalid(void **state)
{
    struct stream_t stream;
    uint8_t buffer[1];

    expect_assert_failure(Stream_Init(NULL, buffer, sizeof(buffer)));
    expect_assert_failure(Stream_Init(&stream, NULL, sizeof(buffer)));
    expect_assert_failure(Stream_Init(&stream, buffer, 0));
}

void test_Stream_Write_Invalid(void **state)
{
    const uint8_t data[] = {1};
    expect_assert_failure(Stream_Write(NULL, data, sizeof(data)));
    expect_assert_failure(Stream_Write(&stream, NULL, sizeof(data)));
}

void test_Stream_Write(void **state)
{
    const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    assert_int_equal(Stream_Write(&stream, data, sizeof(data)), sizeof(data));
    assert_int_equal(Stream_Write(&stream, data, sizeof(data)), sizeof(buffer) - sizeof(data));
    assert_int_equal(Stream_Write(&stream, data, sizeof(data)), 0);
}

void test_Stream_Read_Invalid(void **state)
{
    uint8_t data[1];
    expect_assert_failure(Stream_Read(NULL, data, sizeof(data)));
    expect_assert_failure(Stream_Read(&stream, NULL, sizeof(data)));
}

void test_Stream_Read(void **state)
{
    const uint8_t data_write[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    uint8_t data_read[10];

    assert_int_equal(Stream_Read(&stream, data_read, sizeof(data_read)), 0);

    Stream_Write(&stream, data_write, 8);
    assert_int_equal(Stream_Read(&stream, data_read, sizeof(data_read)), 8);
    assert_memory_equal(data_read, data_write, 8);

    Stream_Write(&stream, data_write, 16);
    assert_int_equal(Stream_Read(&stream, data_read, sizeof(data_read)), sizeof(data_read));
    assert_memory_equal(data_read, data_write, sizeof(data_read));
    assert_int_equal(Stream_Read(&stream, data_read, sizeof(data_read)), 6);
    assert_memory_equal(data_read, data_write + sizeof(data_read), 6);
}

void test_Stream_GetAvailableSpace_Invalid(void **state)
{
    expect_assert_failure(Stream_GetAvailableSpace(NULL));
}

void test_Stream_GetAvailableSpace(void **state)
{
    const uint8_t data[sizeof(buffer)];

    assert_int_equal(Stream_GetAvailableSpace(&stream), sizeof(buffer));

    Stream_Write(&stream, data, 1);
    assert_int_equal(Stream_GetAvailableSpace(&stream), sizeof(buffer) - 1);

    Stream_Write(&stream, data, sizeof(buffer));
    assert_int_equal(Stream_GetAvailableSpace(&stream), 0);
}

void test_Stream_HasData_Invalid(void **state)
{
    expect_assert_failure(Stream_HasData(NULL));
}

void test_Stream_HasData(void **state)
{
    assert_false(Stream_HasData(&stream));

    const uint8_t data_write[1];
    uint8_t data_read[1];

    Stream_Write(&stream, data_write, sizeof(data_write));
    assert_true(Stream_HasData(&stream));

    Stream_Read(&stream, data_read, sizeof(data_read));
    assert_false(Stream_HasData(&stream));
}

void test_Stream_Clear_Invalid(void **state)
{
    expect_assert_failure(Stream_Clear(NULL));
}

void test_Stream_Clear(void **state)
{
    uint8_t data_write = 1;
    uint8_t data_read[4];

    Stream_Clear(&stream);
    assert_false(Stream_HasData(&stream));

    Stream_Write(&stream, &data_write, 1);
    Stream_Clear(&stream);
    assert_false(Stream_HasData(&stream));
    assert_int_equal(Stream_Read(&stream, data_read, sizeof(data_read)), 0);

    /* Write and read after clear to test if head and tail are reset properly. */
    data_write = 2;
    Stream_Write(&stream, &data_write, 1);
    assert_int_equal(Stream_Read(&stream, data_read, sizeof(data_read)), 1);
    assert_memory_equal(data_read, &data_write, 1);
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_stream[] =
    {
        cmocka_unit_test(test_Stream_Init_Invalid),
        cmocka_unit_test_setup(test_Stream_Write_Invalid, Setup),
        cmocka_unit_test_setup(test_Stream_Write, Setup),
        cmocka_unit_test_setup(test_Stream_Read_Invalid, Setup),
        cmocka_unit_test_setup(test_Stream_Read, Setup),
        cmocka_unit_test_setup(test_Stream_GetAvailableSpace_Invalid, Setup),
        cmocka_unit_test_setup(test_Stream_GetAvailableSpace, Setup),
        cmocka_unit_test_setup(test_Stream_HasData_Invalid, Setup),
        cmocka_unit_test_setup(test_Stream_HasData, Setup),
        cmocka_unit_test_setup(test_Stream_Clear_Invalid, Setup),
        cmocka_unit_test_setup(test_Stream_Clear, Setup),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_stream, NULL, NULL);
}
