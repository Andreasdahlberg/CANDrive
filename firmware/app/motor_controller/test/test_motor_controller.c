/**
 * @file   test_motor_controller.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the Motor Controller.
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
#include <stdlib.h>
#include "utility.h"
#include "motor.h"
#include "motor_controller.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define NUMBER_OF_MOTORS 2

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct logging_logger_t *dummy_logger;
const static struct motor_config_t motor_configs[NUMBER_OF_MOTORS];

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static int Setup(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    will_return_maybe(Board_GetNumberOfMotors, NUMBER_OF_MOTORS);
    for (size_t i = 0; i < NUMBER_OF_MOTORS; ++i)
    {
        will_return(Board_GetMotorConfig, &motor_configs[i]);
        expect_value(Motor_Init, config_p, &motor_configs[i]);
    }

    MotorController_Init();
    return 0;
}

static void ExpectPIDUpdate(int32_t rpm_cv, int32_t current_cv, int32_t expected_cv)
{
    const uint32_t pid_update_period = 100;
    will_return(SysTime_GetDifference, pid_update_period);
    expect_function_calls(Motor_Update, NUMBER_OF_MOTORS);

    for (size_t i = 0; i < NUMBER_OF_MOTORS; ++i)
    {
        will_return(Motor_GetRPM, 0);
        will_return(Motor_GetCurrent, 0);

        will_return(PID_Update, rpm_cv);
        will_return(PID_Update, current_cv);

        expect_value(Motor_SetSpeed, speed, expected_cv);
    }
    will_return(SysTime_GetSystemTime, 100);
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_MotorController_Init_UnsupportedNumberOfMotors(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    will_return_maybe(Board_GetNumberOfMotors, NUMBER_OF_MOTORS + 1);

    expect_assert_failure(MotorController_Init());
}

static void test_MotorController_Init(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    will_return_maybe(Board_GetNumberOfMotors, NUMBER_OF_MOTORS);
    for (size_t i = 0; i < NUMBER_OF_MOTORS; ++i)
    {
        will_return(Board_GetMotorConfig, &motor_configs[i]);
        expect_value(Motor_Init, config_p, &motor_configs[i]);
    }

    MotorController_Init();
}

static void test_MotorController_Update(void **state)
{
    will_return_maybe(Board_GetNumberOfMotors, NUMBER_OF_MOTORS);

    /* No PID update */
    will_return(SysTime_GetDifference, 0);
    expect_function_calls(Motor_Update, NUMBER_OF_MOTORS);
    MotorController_Update();

    /* PID update */
    ExpectPIDUpdate(INT16_MIN, 0, INT16_MIN);
    MotorController_Update();
    ExpectPIDUpdate(INT16_MAX, 1, 1);
    MotorController_Update();
    ExpectPIDUpdate(INT16_MAX, INT16_MAX, INT16_MAX);
    MotorController_Update();
}

static void test_MotorController_SetRpm_Invalid(void **state)
{
    will_return_maybe(Board_GetNumberOfMotors, NUMBER_OF_MOTORS);
    expect_assert_failure(MotorController_SetRPM(NUMBER_OF_MOTORS + 1, 0));
}

static void test_MotorController_SetRpm(void **state)
{
    will_return_maybe(Board_GetNumberOfMotors, NUMBER_OF_MOTORS);

    const int16_t data[] = {INT16_MIN, 0, 50, INT16_MAX};
    for (size_t i = 0; i < ElementsIn(data); ++i)
    {
        expect_value(PID_SetSetpoint, setpoint, data[i]);
        MotorController_SetRPM(0, data[i]);
    }
}

static void test_MotorController_SetCurrent_Invalid(void **state)
{
    will_return_maybe(Board_GetNumberOfMotors, NUMBER_OF_MOTORS);
    expect_assert_failure(MotorController_SetCurrent(NUMBER_OF_MOTORS + 1, 0));
}

static void test_MotorController_SetCurrent(void **state)
{
    will_return_maybe(Board_GetNumberOfMotors, NUMBER_OF_MOTORS);

    const int16_t data[] = {INT16_MIN, 0, 50, INT16_MAX};
    for (size_t i = 0; i < ElementsIn(data); ++i)
    {
        expect_value(PID_SetSetpoint, setpoint, data[i]);
        MotorController_SetCurrent(0, data[i]);
    }
}

static void test_MotorController_Coast_Invalid(void **state)
{
    will_return_maybe(Board_GetNumberOfMotors, NUMBER_OF_MOTORS);
    expect_assert_failure(MotorController_Coast(NUMBER_OF_MOTORS + 1));
}

static void test_MotorController_Coast(void **state)
{
    will_return_maybe(Board_GetNumberOfMotors, NUMBER_OF_MOTORS);
    /* Expect assert failure since it's not implemented yet. */
    expect_assert_failure(MotorController_Coast(0));
}

static void test_MotorController_Brake_Invalid(void **state)
{
    will_return_maybe(Board_GetNumberOfMotors, NUMBER_OF_MOTORS);
    expect_assert_failure(MotorController_Brake(NUMBER_OF_MOTORS + 1));
}

static void test_MotorController_Brake(void **state)
{
    will_return_maybe(Board_GetNumberOfMotors, NUMBER_OF_MOTORS);
    /* Expect assert failure since it's not implemented yet. */
    expect_assert_failure(MotorController_Brake(0));
}

static void test_MotorController_GetPosition_Invalid(void **state)
{
    will_return_maybe(Board_GetNumberOfMotors, NUMBER_OF_MOTORS);
    expect_assert_failure(MotorController_GetPosition(NUMBER_OF_MOTORS + 1));
}

static void test_MotorController_GetPosition(void **state)
{
    will_return_maybe(Board_GetNumberOfMotors, NUMBER_OF_MOTORS);

    const uint32_t data[] = {0, UINT32_MAX};
    for (size_t i = 0; i < ElementsIn(data); ++i)
    {
        will_return(Motor_GetPosition, data[i]);
        assert_int_equal(MotorController_GetPosition(0), data[i]);
    }
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_motor_controller[] =
    {
        cmocka_unit_test(test_MotorController_Init_UnsupportedNumberOfMotors),
        cmocka_unit_test(test_MotorController_Init),
        cmocka_unit_test_setup(test_MotorController_Update, Setup),
        cmocka_unit_test_setup(test_MotorController_SetRpm_Invalid, Setup),
        cmocka_unit_test_setup(test_MotorController_SetRpm, Setup),
        cmocka_unit_test_setup(test_MotorController_SetCurrent_Invalid, Setup),
        cmocka_unit_test_setup(test_MotorController_SetCurrent, Setup),
        cmocka_unit_test_setup(test_MotorController_Coast_Invalid, Setup),
        cmocka_unit_test_setup(test_MotorController_Coast, Setup),
        cmocka_unit_test_setup(test_MotorController_Brake_Invalid, Setup),
        cmocka_unit_test_setup(test_MotorController_Brake, Setup),
        cmocka_unit_test_setup(test_MotorController_GetPosition_Invalid, Setup),
        cmocka_unit_test_setup(test_MotorController_GetPosition, Setup),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_motor_controller, NULL, NULL);
}
