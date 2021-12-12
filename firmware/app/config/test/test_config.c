/**
 * @file   test_config.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the config module.
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
#include "config.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct test_config_t
{
    uint32_t number_of_motors;
    uint32_t counts_per_rev;
    uint32_t no_load_rpm;
    uint32_t no_load_current;
    uint32_t stall_current;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_Config_Valid(void **state)
{
    struct test_config_t config =
    {
        .number_of_motors = 2,
        .counts_per_rev = 4800,
        .no_load_rpm = 120,
        .no_load_current = 200,
        .stall_current = 3000
    };

    will_return(NVS_Retrieve, config.number_of_motors);
    will_return(NVS_Retrieve, true);
    will_return(NVS_Retrieve, config.counts_per_rev);
    will_return(NVS_Retrieve, true);
    will_return(NVS_Retrieve, config.no_load_rpm);
    will_return(NVS_Retrieve, true);
    will_return(NVS_Retrieve, config.no_load_current);
    will_return(NVS_Retrieve, true);
    will_return(NVS_Retrieve, config.stall_current);
    will_return(NVS_Retrieve, true);

    Config_Init();

    assert_true(Config_IsValid());
    assert_int_equal(Config_GetNumberOfMotors(), config.number_of_motors);
    assert_int_equal(Config_GetCountsPerRev(), config.counts_per_rev);
    assert_int_equal(Config_GetNoLoadRpm(), config.no_load_rpm);
    assert_int_equal(Config_GetNoLoadCurrent(), config.no_load_current);
    assert_int_equal(Config_GetStallCurrent(), config.stall_current);
}

static void test_Config_Invalid(void **state)
{
    size_t number_of_parameters = 5;
    for (size_t i = 0; i < number_of_parameters; ++i)
    {
        for (size_t n = 0; n < i; ++n)
        {
            will_return(NVS_Retrieve, 0);
            will_return(NVS_Retrieve, true);
        }

        will_return(NVS_Retrieve, 0);
        will_return(NVS_Retrieve, false);

        Config_Init();
        assert_false(Config_IsValid());
    }
}

static void test_Config_Invalid_ParameterZeroCheck(void **state)
{
    will_return(NVS_Retrieve, 2);
    will_return(NVS_Retrieve, true);
    will_return(NVS_Retrieve, 3);
    will_return(NVS_Retrieve, true);
    will_return(NVS_Retrieve, 4);
    will_return(NVS_Retrieve, false);

    Config_Init();

    assert_false(Config_IsValid());
    assert_int_equal(Config_GetNumberOfMotors(), 0);
    assert_int_equal(Config_GetCountsPerRev(), 0);
    assert_int_equal(Config_GetNoLoadRpm(), 0);
    assert_int_equal(Config_GetNoLoadCurrent(), 0);
    assert_int_equal(Config_GetStallCurrent(), 0);
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_config[] =
    {
        cmocka_unit_test(test_Config_Valid),
        cmocka_unit_test(test_Config_Invalid),
        cmocka_unit_test(test_Config_Invalid_ParameterZeroCheck),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_config, NULL, NULL);
}
