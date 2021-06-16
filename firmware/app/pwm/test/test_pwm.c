/**
 * @file   test_pwm.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the PWM module.
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
#include <libopencm3/stm32/timer.h>
#include "utility.h"
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

static const uint32_t channel_to_timer_map[1] = {TIM3};
static pwm_output_t pwm_output;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static int Setup(void **state)
{
    const size_t channel = 0;
    pwm_output = (__typeof__(pwm_output)) {0};

    expect_any(timer_continuous_mode, timer_peripheral);
    expect_any(timer_set_oc_mode, timer_peripheral);
    expect_any(timer_set_oc_mode, oc_mode);

    PWM_Init(&pwm_output, channel);
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

void test_PWM_Init_InvalidParameters(void **state)
{
    pwm_output_t output;
    expect_assert_failure(PWM_Init(NULL, 0));
    expect_assert_failure(PWM_Init(&output, 1));
    expect_assert_failure(PWM_Init(NULL, 1));
}

void test_PWM_Init(void **state)
{
    const size_t channel = 0;
    pwm_output_t output;

    expect_value(timer_continuous_mode, timer_peripheral, channel_to_timer_map[channel]);
    expect_value(timer_set_oc_mode, timer_peripheral, channel_to_timer_map[channel]);
    expect_value(timer_set_oc_mode, oc_mode, TIM_OCM_PWM1);

    PWM_Init(&output, channel);
}

static void test_PWM_SetFrequency_InvalidParameters(void **state)
{
    const uint32_t main_frequency = 72000000;

    expect_assert_failure(PWM_SetFrequency(NULL, 1));
    expect_assert_failure(PWM_SetFrequency(&pwm_output, 0));
    expect_assert_failure(PWM_SetFrequency(&pwm_output, main_frequency + 1));
}

static void test_PWM_SetFrequency(void **state)
{
    const size_t channel = 0;
    const uint32_t main_frequency = 72000000;

    const uint32_t frequencies[] = {1, 20000, main_frequency};
    for (size_t i = 0; i < ElementsIn(frequencies); ++i)
    {
        const uint32_t expected_period = main_frequency / frequencies[i];
        expect_value(timer_set_period, timer_peripheral, channel_to_timer_map[channel]);
        expect_value(timer_set_period, period, expected_period);
        PWM_SetFrequency(&pwm_output, frequencies[i]);
    }
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_PWM[] =
    {
        cmocka_unit_test(test_PWM_Init_InvalidParameters),
        cmocka_unit_test(test_PWM_Init),
        cmocka_unit_test_setup(test_PWM_SetFrequency_InvalidParameters, Setup),
        cmocka_unit_test_setup(test_PWM_SetFrequency, Setup),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_PWM, NULL, NULL);
}

