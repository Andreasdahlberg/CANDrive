/**
 * @file   test_utility.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the utility module.
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
#include "utility.h"

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

static void test_CopyString(void **state)
{
    char destination[16];
    char source[6] = "hello";

    size_t result = CopyString(destination, source, sizeof(destination));
    assert_int_equal(result, strlen(source));
    assert_string_equal(destination, source);
}

static void test_CopyString_Empty(void **state)
{
    char destination[16];
    char source[1] = "";

    size_t result = CopyString(destination, source, sizeof(destination));
    assert_int_equal(result, strlen(source));
    assert_string_equal(destination, source);
}

static void test_CopyString_Truncated(void **state)
{
    char destination[11];
    char source[12] = "hello world";

    size_t result = CopyString(destination, source, sizeof(destination));
    assert_int_equal(result, sizeof(destination) - 1);
    assert_string_equal(destination, "hello worl");
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_utility[] =
    {
        cmocka_unit_test(test_CopyString),
        cmocka_unit_test(test_CopyString_Empty),
        cmocka_unit_test(test_CopyString_Truncated),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_utility, NULL, NULL);
}
