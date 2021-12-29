/**
 * @file   test_pid.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the PID controller module.
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
#include "pid.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct pid_t pid;
static struct pid_parameters_t parameters;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static int Setup(void **state)
{
    PID_Init(&pid);

    parameters.kp = 10;
    parameters.ki = 10;
    parameters.kd = 1;
    parameters.imax = 1000;
    parameters.imin = -1000;
    parameters.cvmax = 100;
    parameters.cvmin = -100;
    parameters.scale = 100;

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_PID_Init_Invalid(void **state)
{
    expect_assert_failure(PID_Init(NULL));
}

static void test_PID_Update_Invalid(void **state)
{
    expect_assert_failure(PID_Update(NULL, 0));
}

static void test_PID_Update_PositiveControlVariableLimit(void **state)
{
    assert_int_equal(PID_GetOutput(&pid), 0);

    const int32_t cv_limits[] = {0, 1, 100};
    for (size_t i = 0; i < ElementsIn(cv_limits); ++i)
    {
        parameters.cvmax = cv_limits[i];

        PID_SetParameters(&pid, &parameters);
        PID_SetSetpoint(&pid, 50);

        for (size_t i = 0; i < 50; ++i)
        {
            PID_Update(&pid, 0);
        }

        assert_int_equal(PID_GetOutput(&pid), parameters.cvmax);
    }
}

static void test_PID_Update_NegativeControlVariableLimit(void **state)
{
    assert_int_equal(PID_GetOutput(&pid), 0);

    const int32_t cv_limits[] = {0, -1, -100};
    for (size_t i = 0; i < ElementsIn(cv_limits); ++i)
    {
        parameters.cvmin = cv_limits[i];

        PID_SetParameters(&pid, &parameters);
        PID_SetSetpoint(&pid, -50);

        for (size_t i = 0; i < 50; ++i)
        {
            PID_Update(&pid, 0);
        }

        assert_int_equal(PID_GetOutput(&pid), parameters.cvmin);
    }
}

static void test_PID_SetSetpoint_Invalid(void **state)
{
    expect_assert_failure(PID_SetSetpoint(NULL, 0));
}

static void test_PID_GetSetpoint_Invalid(void **state)
{
    expect_assert_failure(PID_GetSetpoint(NULL));
}

static void test_PID_SetAndGetSetpoint(void **state)
{
    const int32_t data[] = {INT32_MIN, -1, 0, 1, INT32_MAX};

    for (size_t i = 0; i < ElementsIn(data); ++i)
    {
        PID_SetSetpoint(&pid, data[i]);
        assert_int_equal(PID_GetSetpoint(&pid), data[i]);
    }
}

static void test_PID_SetParameters_Invalid(void **state)
{
    struct pid_t pid;
    struct pid_parameters_t parameters;

    expect_assert_failure(PID_SetParameters(NULL, &parameters));
    expect_assert_failure(PID_SetParameters(&pid, NULL));
}

static void test_PID_GetParameters_Invalid(void **state)
{
    expect_assert_failure(PID_GetParameters(NULL));
}

static void test_PID_SetGetParameters(void **state)
{
    PID_SetParameters(&pid, &parameters);

    struct pid_parameters_t *set_parameters = PID_GetParameters(&pid);
    assert_int_equal(set_parameters->kp, parameters.kp);
    assert_int_equal(set_parameters->ki, parameters.ki);
    assert_int_equal(set_parameters->kd, parameters.kd);
    assert_int_equal(set_parameters->imax, parameters.imax);
    assert_int_equal(set_parameters->imin, parameters.imin);
    assert_int_equal(set_parameters->cvmax, parameters.cvmax);
    assert_int_equal(set_parameters->cvmin, parameters.cvmin);
    assert_int_equal(set_parameters->scale, parameters.scale);
}

static void test_PID_GetOutput_Invalid(void **state)
{
    expect_assert_failure(PID_GetOutput(NULL));
}

static void test_PID_Reset_Invalid(void **state)
{
    expect_assert_failure(PID_Reset(NULL));
}

static void test_PID_Reset(void **state)
{
    PID_SetParameters(&pid, &parameters);
    PID_SetSetpoint(&pid, 100);
    PID_Update(&pid, 0);
    assert_int_not_equal(PID_GetOutput(&pid), 0);

    PID_Reset(&pid);
    assert_int_equal(PID_GetOutput(&pid), 0);
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_PID[] =
    {
        cmocka_unit_test(test_PID_Init_Invalid),
        cmocka_unit_test(test_PID_Update_Invalid),
        cmocka_unit_test_setup(test_PID_Update_PositiveControlVariableLimit, Setup),
        cmocka_unit_test_setup(test_PID_Update_NegativeControlVariableLimit, Setup),
        cmocka_unit_test(test_PID_SetSetpoint_Invalid),
        cmocka_unit_test(test_PID_GetSetpoint_Invalid),
        cmocka_unit_test(test_PID_SetAndGetSetpoint),
        cmocka_unit_test(test_PID_SetParameters_Invalid),
        cmocka_unit_test(test_PID_GetParameters_Invalid),
        cmocka_unit_test_setup(test_PID_SetGetParameters, Setup),
        cmocka_unit_test(test_PID_GetOutput_Invalid),
        cmocka_unit_test(test_PID_Reset_Invalid),
        cmocka_unit_test(test_PID_Reset),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_PID, NULL, NULL);
}
