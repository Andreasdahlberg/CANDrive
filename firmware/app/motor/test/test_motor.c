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
#include <stdlib.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include "utility.h"
#include "motor.h"
#include "pwm.h"
#include "adc.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

const static struct motor_config_t motor_config =
{
    .pwm = {
        .timer_peripheral = TIM3,
        .remap = AFIO_MAPR_TIM3_REMAP_FULL_REMAP,
        .gpio_port = GPIOC,
        .gpio = GPIO8,
        .oc_id = TIM_OC3,
        .peripheral_clocks = {RCC_GPIOC, RCC_TIM3, RCC_AFIO}
    },
    .driver = {
        .port = GPIOC,
        .sel = GPIO0,
        .cs = GPIO1,
        .ina = GPIO2,
        .inb = GPIO3,
        .gpio_clock = RCC_GPIOC
    },
    .encoder = {
        .port = GPIOB,
        .a = GPIO6,
        .b = GPIO7,
        .gpio_clock = RCC_GPIOB,
        .timer = TIM4,
        .timer_clock = RCC_TIM4,
        .timer_rst = RST_TIM4
    },
    .adc = {
        .channel = 11
    }
};

static struct logging_logger_t *dummy_logger;
const uint32_t DEFAULT_PWM_FREQUENCY = 20000;
struct motor_t motor;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static int Setup(void **state)
{
    const char motor_name[] = "TM1";
    motor = (__typeof__(motor)) {0};

    will_return(Logging_GetLogger, dummy_logger);
    expect_value(timer_set_period, timer_peripheral, motor_config.encoder.timer);
    expect_value(timer_set_period, period, 29);
    expect_value(timer_enable_counter, timer_peripheral, motor_config.encoder.timer);
    expect_value(PWM_Init, config_p, &motor_config.pwm);
    expect_function_call(PWM_Disable);
    expect_value(PWM_SetFrequency, frequency, DEFAULT_PWM_FREQUENCY);
    expect_value(PWM_SetDuty, duty, 0);

    Motor_Init(&motor, motor_name, &motor_config);
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_Motor_Init_Error(void **state)
{
    const char motor_name[] = "TM1";

    expect_assert_failure(Motor_Init(NULL, motor_name, &motor_config));
    expect_assert_failure(Motor_Init(&motor, NULL, &motor_config));
    expect_assert_failure(Motor_Init(&motor, motor_name, NULL));
}

static void test_Motor_Init(void **state)
{
    const char motor_name[] = "TM1";
    pwm_output_t pwm;
    adc_input_t adc;

    will_return(Logging_GetLogger, dummy_logger);
    expect_value(timer_set_period, timer_peripheral, motor_config.encoder.timer);
    expect_value(timer_set_period, period, 29);
    expect_value(timer_enable_counter, timer_peripheral, motor_config.encoder.timer);
    expect_value(PWM_Init, config_p, &motor_config.pwm);
    expect_function_call(PWM_Disable);
    expect_value(PWM_SetFrequency, frequency, DEFAULT_PWM_FREQUENCY);
    expect_value(PWM_SetDuty, duty, 0);

    Motor_Init(&motor, motor_name, &motor_config);
}

static void test_Motor_GetRPM_Invalid(void **state)
{
    expect_assert_failure(Motor_GetRPM(NULL));
}

static void test_Motor_GetRPM(void **state)
{
    assert_int_equal(Motor_GetRPM(&motor), 0);
}

static void test_Motor_GetCurrent_Invalid(void **state)
{
    expect_assert_failure(Motor_GetCurrent(NULL));
}

static void test_Motor_GetCurrent(void **state)
{
    const uint32_t current_sense_voltages[] = {0, 1, 2000, INT16_MAX - 1};

    /* Clockwise -> positive current */
    for (size_t i = 0; i < ElementsIn(current_sense_voltages); ++i)
    {
        will_return(ADC_GetVoltage, current_sense_voltages[i]);
        expect_value(timer_get_direction, timer_peripheral, motor_config.encoder.timer);
        will_return(timer_get_direction, TIM_CR1_DIR_UP);

        int16_t current = current_sense_voltages[i];
        assert_int_equal(Motor_GetCurrent(&motor), current);
    }

    /* Counter clockwise -> negative current */
    for (size_t i = 0; i < ElementsIn(current_sense_voltages); ++i)
    {
        will_return(ADC_GetVoltage, current_sense_voltages[i]);
        expect_value(timer_get_direction, timer_peripheral, motor_config.encoder.timer);
        will_return(timer_get_direction, TIM_CR1_DIR_DOWN);

        int16_t current = current_sense_voltages[i] * -1;
        assert_int_equal(Motor_GetCurrent(&motor), current);
    }
}

static void test_Motor_SetSpeed_Invalid(void **state)
{
    expect_assert_failure(Motor_SetSpeed(NULL, 0));

    int16_t invalid_speeds[] = {INT16_MIN, -101, 101, INT16_MAX};
    for (size_t i = 0; i < ElementsIn(invalid_speeds); ++i)
    {
        expect_assert_failure(Motor_SetSpeed(&motor, invalid_speeds[i]));
    }
}

static void test_Motor_SetSpeed(void **state)
{
    int16_t speeds[] = {-100, -25, 0, 50, 100};
    for (size_t i = 0; i < ElementsIn(speeds); ++i)
    {
        const int16_t speed = speeds[i];

        expect_function_call(PWM_Disable);
        expect_value(PWM_SetDuty, duty, abs(speed));
        expect_function_call(PWM_Enable);

        Motor_SetSpeed(&motor, speed);
    }
}

static void test_Motor_Coast_Invalid(void **state)
{
    expect_assert_failure(Motor_Coast(NULL));
}

static void test_Motor_Coast(void **state)
{
    expect_value(PWM_SetDuty, duty, 0);
    Motor_Coast(&motor);
}

static void test_Motor_Run(void **state)
{
    const int16_t speed = 75;

    /* Set speed */
    expect_function_call(PWM_Disable);
    expect_value(PWM_SetDuty, duty, speed);
    expect_function_call(PWM_Enable);
    Motor_SetSpeed(&motor, speed);

    /* Coast */
    expect_value(PWM_SetDuty, duty, 0);
    Motor_Coast(&motor);

    /*Run motor again and expect the set speed again. */
    expect_function_call(PWM_Disable);
    expect_value(PWM_SetDuty, duty, speed);
    expect_function_call(PWM_Enable);
    Motor_Run(&motor);
}

static void test_Motor_Brake_Invalid(void **state)
{
    expect_assert_failure(Motor_Brake(NULL));
}

static void test_Motor_Brake(void **state)
{
    expect_function_call(PWM_Disable);
    expect_value(PWM_SetDuty, duty, 100);
    expect_function_call(PWM_Enable);
    Motor_Brake(&motor);
}

static void test_Motor_GetStatus_Invalid(void **state)
{
    expect_assert_failure(Motor_GetStatus(NULL));
}

static void test_Motor_GetStatus(void **state)
{
    assert_int_equal(Motor_GetStatus(&motor), MOTOR_UNKNOWN);
}

static void test_Motor_GetDirection_Invalid(void **state)
{
    expect_assert_failure(Motor_GetDirection(NULL));
}

static void test_Motor_GetDirection(void **state)
{
    expect_value(timer_get_direction, timer_peripheral, motor_config.encoder.timer);
    will_return(timer_get_direction, TIM_CR1_DIR_UP);
    assert_int_equal(Motor_GetDirection(&motor), MOTOR_DIR_CW);

    expect_value(timer_get_direction, timer_peripheral, motor_config.encoder.timer);
    will_return(timer_get_direction, TIM_CR1_DIR_DOWN);
    assert_int_equal(Motor_GetDirection(&motor), MOTOR_DIR_CCW);
}

static void test_Motor_GetPosition_Invalid(void **state)
{
    expect_assert_failure(Motor_GetPosition(NULL));
}

static void test_Motor_GetPosition(void **state)
{
    uint32_t counter_data[] = {0, 1, 28, 29};

    for (size_t i = 0; i < ElementsIn(counter_data); ++i)
    {
        expect_value(timer_get_counter, timer_peripheral, motor_config.encoder.timer);
        will_return(timer_get_counter, counter_data[i]);

        uint32_t expect_position = (counter_data[i] * 360) / 30;
        assert_int_equal(Motor_GetPosition(&motor), expect_position);
    }
}

static void test_Motor_DirectionToString(void **state)
{
    assert_string_equal(Motor_DirectionToString(&motor, MOTOR_DIR_CW), "MOTOR_DIR_CW");
    assert_string_equal(Motor_DirectionToString(&motor, MOTOR_DIR_CCW), "MOTOR_DIR_CCW");
    assert_string_equal(Motor_DirectionToString(&motor, UINT32_MAX), "UNKNOWN");
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_FIFO[] =
    {
        cmocka_unit_test(test_Motor_Init_Error),
        cmocka_unit_test(test_Motor_Init),
        cmocka_unit_test_setup(test_Motor_GetRPM_Invalid, Setup),
        cmocka_unit_test_setup(test_Motor_GetRPM, Setup),
        cmocka_unit_test_setup(test_Motor_GetCurrent_Invalid, Setup),
        cmocka_unit_test_setup(test_Motor_GetCurrent, Setup),
        cmocka_unit_test_setup(test_Motor_SetSpeed_Invalid, Setup),
        cmocka_unit_test_setup(test_Motor_SetSpeed, Setup),
        cmocka_unit_test_setup(test_Motor_Coast_Invalid, Setup),
        cmocka_unit_test_setup(test_Motor_Coast, Setup),
        cmocka_unit_test_setup(test_Motor_Run, Setup),
        cmocka_unit_test_setup(test_Motor_Brake_Invalid, Setup),
        cmocka_unit_test_setup(test_Motor_Brake, Setup),
        cmocka_unit_test_setup(test_Motor_GetStatus_Invalid, Setup),
        cmocka_unit_test_setup(test_Motor_GetStatus, Setup),
        cmocka_unit_test_setup(test_Motor_GetDirection_Invalid, Setup),
        cmocka_unit_test_setup(test_Motor_GetDirection, Setup),
        cmocka_unit_test_setup(test_Motor_GetPosition_Invalid, Setup),
        cmocka_unit_test_setup(test_Motor_GetPosition, Setup),
        cmocka_unit_test_setup(test_Motor_DirectionToString, Setup),

    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_FIFO, NULL, NULL);
}
