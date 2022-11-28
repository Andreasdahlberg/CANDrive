/**
 * @file   test_board.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the board module.
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
#include "board.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

uintptr_t __approm_start__;
uintptr_t __approm_size__;
uintptr_t __uprom_start__;
uintptr_t __uprom_size__;
uintptr_t __nvsrom_start__;
uintptr_t __nvsrom_size__;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_Board_GetHardwareRevision(void **state)
{
    assert_int_equal(Board_GetHardwareRevision(), 0);
}

static void test_Board_GetSoftwareRevision(void **state)
{
    assert_int_equal(Board_GetSoftwareRevision(), 0);
}

static void test_Board_GetMotorConfig_Invalid(void **state)
{
    const size_t max_number_of_motors = Board_GetMaxNumberOfMotors();
    expect_assert_failure(Board_GetMotorConfig(max_number_of_motors));
    expect_assert_failure(Board_GetMotorConfig(SIZE_MAX));
}

static void test_Board_GetMotorConfig(void **state)
{
    const size_t max_number_of_motors = Board_GetMaxNumberOfMotors();

    for (size_t i = 0; i < max_number_of_motors; ++i)
    {
        assert_non_null(Board_GetMotorConfig(i));
    }
}

static void test_Board_ToggleStatusLED(void **state)
{
    expect_function_call(gpio_toggle);
    Board_ToggleStatusLED();
}

static void test_Board_GetEmergencyPinState(void **state)
{
    will_return(gpio_get, false);
    assert_true(Board_GetEmergencyPinState());

    will_return(gpio_get, true);
    assert_false(Board_GetEmergencyPinState());
}

static void test_Board_GetNVSAddress(void **state)
{
    assert_int_equal(Board_GetNVSAddress(), &__nvsrom_start__);
}

static void test_Board_GetNumberOfPagesInNVS(void **state)
{
    skip();
    assert_int_equal(Board_GetNumberOfPagesInNVS(), 2);
}

static void test_Board_GetMaxCurrent(void **state)
{
    assert_int_equal(Board_GetMaxCurrent(), 5000);
}

static void test_Board_VSenseToVoltage(void **state)
{
    assert_int_equal(Board_VSenseToVoltage(0), 0);
    assert_int_equal(Board_VSenseToVoltage(1650), 15400);
    assert_int_equal(Board_VSenseToVoltage(3300), 30800);
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_board[] =
    {
        cmocka_unit_test(test_Board_GetHardwareRevision),
        cmocka_unit_test(test_Board_GetSoftwareRevision),
        cmocka_unit_test(test_Board_GetMotorConfig_Invalid),
        cmocka_unit_test(test_Board_GetMotorConfig),
        cmocka_unit_test(test_Board_ToggleStatusLED),
        cmocka_unit_test(test_Board_GetEmergencyPinState),
        cmocka_unit_test(test_Board_GetNVSAddress),
        cmocka_unit_test(test_Board_GetNumberOfPagesInNVS),
        cmocka_unit_test(test_Board_GetMaxCurrent),
        cmocka_unit_test(test_Board_VSenseToVoltage)
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_board, NULL, NULL);
}
