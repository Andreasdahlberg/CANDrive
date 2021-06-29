/**
 * @file   test_motor.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the Motor module.
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

#include "motor.h"
#include "pwm.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct logging_logger_t *dummy_logger;
const uint32_t DEFAULT_PWM_FREQUENCY = 20000;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static int Setup(void **state)
{
    will_return(Logging_GetLogger, dummy_logger);

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_Motor_Init_Error(void **state)
{
    const char motor_name[] = "TM1";
    struct motor_t motor;
    struct pwm_output_t pwm;

    expect_assert_failure(Motor_Init(NULL, motor_name, &pwm));
    expect_assert_failure(Motor_Init(&motor, NULL, &pwm));
    expect_assert_failure(Motor_Init(&motor, motor_name, NULL));
}

static void test_Motor_Init(void **state)
{
    const char motor_name[] = "TM1";
    struct motor_t motor;
    struct pwm_output_t pwm;

    will_return(Logging_GetLogger, dummy_logger);
    expect_function_call(PWM_Disable);
    expect_value(PWM_SetFrequency, frequency, DEFAULT_PWM_FREQUENCY);
    expect_value(PWM_SetDuty, duty, 0);

    Motor_Init(&motor, motor_name, &pwm);
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_FIFO[] =
    {
        cmocka_unit_test(test_Motor_Init_Error),
        cmocka_unit_test(test_Motor_Init)
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_FIFO, NULL, NULL);
}
