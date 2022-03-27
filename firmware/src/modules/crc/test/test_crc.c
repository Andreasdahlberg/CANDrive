/**
 * @file   test_crc.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the CRC module.
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
#include "utility.h"
#include "crc.h"

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

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_CRC_Calculate_Even(void **state)
{
    uint32_t data[2] = {0xAABBCCDD, 0xEEFFAABB};

    expect_function_call(crc_reset);

    for (size_t i = 0; i < ElementsIn(data); ++i)
    {
        expect_value(crc_calculate, data, data[i]);
        will_return(crc_calculate, i);
    }

    const uint32_t result = CRC_Calculate(data, sizeof(data));
    assert_int_equal(result, 1);
}

static void test_CRC_Calculate_Uneven(void **state)
{
    uint8_t data[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

    expect_function_call(crc_reset);

    expect_value(crc_calculate, data, 0xDDCCBBAA);
    will_return(crc_calculate, 10);

    expect_value(crc_calculate, data, 0xFFEE);
    will_return(crc_calculate, 20);

    const uint32_t result = CRC_Calculate(data, sizeof(data));
    assert_int_equal(result, 20);
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_crc[] =
    {
        cmocka_unit_test(test_CRC_Calculate_Even),
        cmocka_unit_test(test_CRC_Calculate_Uneven),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_crc, NULL, NULL);
}
