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
#include <libopencm3/stm32/gpio.h>
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

static const struct pwm_config_t pwm_config =
{
    .timer_peripheral = TIM3,
    .remap = AFIO_MAPR_TIM3_REMAP_FULL_REMAP,
    .gpio_port = GPIOC,
    .gpio = GPIO8,
    .oc_id = TIM_OC3,
    .peripheral_clocks = {RCC_GPIOC, RCC_TIM3, RCC_AFIO}
};

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

    PWM_Init(&pwm_output, &pwm_config);
    return 0;
}

static uint32_t FrequencyToPeriod(uint32_t frequency)
{
    const uint32_t main_frequency = 72000000;
    return main_frequency / frequency;
}

static void SetFrequency(uint32_t frequency)
{
    expect_any(timer_set_period, timer_peripheral);
    expect_any(timer_set_period, period);
    PWM_SetFrequency(&pwm_output, frequency);
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

void test_PWM_Init_InvalidParameters(void **state)
{
    pwm_output_t output;
    expect_assert_failure(PWM_Init(NULL, &pwm_config));
    expect_assert_failure(PWM_Init(&output, NULL));
    expect_assert_failure(PWM_Init(NULL, NULL));
}

void test_PWM_Init(void **state)
{
    pwm_output_t output;

    expect_value(timer_continuous_mode, timer_peripheral, pwm_config.timer_peripheral);
    expect_value(timer_set_oc_mode, timer_peripheral, pwm_config.timer_peripheral);
    expect_value(timer_set_oc_mode, oc_mode, TIM_OCM_PWM1);

    PWM_Init(&output, &pwm_config);
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
    const uint32_t frequencies[] = {1, 20000, 72000000};
    for (size_t i = 0; i < ElementsIn(frequencies); ++i)
    {
        const uint32_t expected_period = FrequencyToPeriod(frequencies[i]);
        expect_value(timer_set_period, timer_peripheral, pwm_config.timer_peripheral);
        expect_value(timer_set_period, period, expected_period);
        PWM_SetFrequency(&pwm_output, frequencies[i]);
    }
}

static void test_PWM_SetDuty_InvalidParameters(void **state)
{
    expect_assert_failure(PWM_SetDuty(NULL, 0));
    expect_assert_failure(PWM_SetDuty(&pwm_output, 101));
    expect_assert_failure(PWM_SetDuty(NULL, 101));
}

static void test_PWM_SetDuty(void **state)
{
    const uint32_t duty_cycles[] = {0, 1, 50, 99, 100};

    SetFrequency(720000);
    for (size_t i = 0; i < ElementsIn(duty_cycles); ++i)
    {
        expect_value(timer_set_oc_value, value, duty_cycles[i]);
        PWM_SetDuty(&pwm_output, duty_cycles[i]);
    }

    SetFrequency(360000);
    for (size_t i = 0; i < ElementsIn(duty_cycles); ++i)
    {
        expect_value(timer_set_oc_value, value, duty_cycles[i] * 2);
        PWM_SetDuty(&pwm_output, (duty_cycles[i]));
    }
}

static void test_PWM_Enable_InvalidParameter(void **state)
{
    expect_assert_failure(PWM_Enable(NULL));
}

static void test_PWM_Enable(void **state)
{
    expect_value(timer_enable_oc_output, timer_peripheral, pwm_config.timer_peripheral);
    expect_value(timer_enable_oc_output, oc_id, TIM_OC3);
    expect_value(timer_enable_counter, timer_peripheral, pwm_config.timer_peripheral);
    PWM_Enable(&pwm_output);
}

static void test_PWM_Disable_InvalidParameter(void **state)
{
    expect_assert_failure(PWM_Disable(NULL));
}

static void test_PWM_Disable(void **state)
{
    expect_value(timer_disable_counter, timer_peripheral, pwm_config.timer_peripheral);
    expect_value(timer_disable_oc_output, timer_peripheral, pwm_config.timer_peripheral);
    expect_value(timer_disable_oc_output, oc_id, TIM_OC3);
    PWM_Disable(&pwm_output);
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
        cmocka_unit_test_setup(test_PWM_SetDuty_InvalidParameters, Setup),
        cmocka_unit_test_setup(test_PWM_SetDuty, Setup),
        cmocka_unit_test_setup(test_PWM_Enable_InvalidParameter, Setup),
        cmocka_unit_test_setup(test_PWM_Enable, Setup),
        cmocka_unit_test_setup(test_PWM_Disable_InvalidParameter, Setup),
        cmocka_unit_test_setup(test_PWM_Disable, Setup)
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_PWM, NULL, NULL);
}

