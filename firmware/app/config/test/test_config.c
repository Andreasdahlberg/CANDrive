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
    uint32_t kp;
    uint32_t ki;
    uint32_t kd;
    uint32_t imax;
    uint32_t imin;
    uint32_t rx_id;
    uint32_t tx_id;
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

static int Setup(void **state)
{
    size_t number_of_parameters = 12;
    for (size_t i = 0; i < number_of_parameters; ++i)
    {
        will_return(NVS_Retrieve, 2);
        will_return(NVS_Retrieve, true);
    }
    Config_Init();

    return 0;
}

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
        .stall_current = 3000,
        .kp = 50,
        .ki = 60,
        .kd = 10,
        .imax = 200,
        .imin = -150,
        .rx_id = 0x001,
        .tx_id = 0x002
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
    will_return(NVS_Retrieve, config.kp);
    will_return(NVS_Retrieve, true);
    will_return(NVS_Retrieve, config.ki);
    will_return(NVS_Retrieve, true);
    will_return(NVS_Retrieve, config.kd);
    will_return(NVS_Retrieve, true);
    will_return(NVS_Retrieve, config.imax);
    will_return(NVS_Retrieve, true);
    will_return(NVS_Retrieve, config.imin);
    will_return(NVS_Retrieve, true);
    will_return(NVS_Retrieve, config.rx_id);
    will_return(NVS_Retrieve, true);
    will_return(NVS_Retrieve, config.tx_id);
    will_return(NVS_Retrieve, true);

    Config_Init();

    assert_true(Config_IsValid());
    assert_int_equal(Config_GetNumberOfMotors(), config.number_of_motors);
    assert_int_equal(Config_GetCountsPerRev(), config.counts_per_rev);
    assert_int_equal(Config_GetNoLoadRpm(), config.no_load_rpm);
    assert_int_equal(Config_GetNoLoadCurrent(), config.no_load_current);
    assert_int_equal(Config_GetStallCurrent(), config.stall_current);
    assert_int_equal(Config_GetValue("kp"), config.kp);
    assert_int_equal(Config_GetValue("ki"), config.ki);
    assert_int_equal(Config_GetValue("kd"), config.kd);
    assert_int_equal(Config_GetValue("imax"), config.imax);
    assert_int_equal(Config_GetValue("imin"), config.imin);
    assert_int_equal(Config_GetValue("rx_id"), config.rx_id);
    assert_int_equal(Config_GetValue("tx_id"), config.tx_id);
}

static void test_Config_Invalid(void **state)
{
    size_t number_of_parameters = 10;
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
    assert_int_equal(Config_GetValue("kp"), 0);
    assert_int_equal(Config_GetValue("ki"), 0);
    assert_int_equal(Config_GetValue("kd"), 0);
    assert_int_equal(Config_GetValue("imax"), 0);
    assert_int_equal(Config_GetValue("imin"), 0);
    assert_int_equal(Config_GetValue("rx_id"), 0);
    assert_int_equal(Config_GetValue("tx_id"), 0);
}

static void test_Config_GetValue_NULL(void **state)
{
    expect_assert_failure(Config_GetValue(NULL));
}

static void test_Config_GetValue(void **state)
{
    /* Invalid name */
    assert_int_equal(Config_GetValue("Foo"), 0);

    /* Valid name */
    assert_int_equal(Config_GetValue("number_of_motors"), 2);
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
        cmocka_unit_test_setup(test_Config_GetValue_NULL, Setup),
        cmocka_unit_test_setup(test_Config_GetValue, Setup),

    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_config, NULL, NULL);
}
