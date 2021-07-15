/**
 * @file   motor.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Motor module.
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

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <assert.h>
#include "utility.h"
#include "motor.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#ifndef MOTOR_LOGGER_DEBUG_LEVEL
#define MOTOR_LOGGER_DEBUG_LEVEL LOGGING_DEBUG
#endif

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static const uint32_t PWM_FREQUENCY = 20000;
static const uint32_t GPIO_PORT = GPIOC;
static const uint16_t GPIO_SEL = GPIO0;
static const uint16_t GPIO_CS = GPIO1;
static const uint16_t GPIO_INA = GPIO2;
static const uint16_t GPIO_INB = GPIO3;
static const enum rcc_periph_clken ENCODER_CLKEN = RCC_TIM4;
static const uint32_t ENCODER_TIMER_PERIPHERAL = TIM4;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static inline void SetupGPIO(void);
static inline void SetupTimer(void);
static inline uint16_t SpeedToDutyCycle(int16_t speed);
static inline void SetGpio(uint16_t gpio, bool state);
static inline void SetDirection(enum motor_direction_t direction);
static inline uint32_t GetPosition(void);
static inline void ResetPosition(void);
static inline uint32_t GetDIRValue(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void Motor_Init(struct motor_t *self_p, const char *name, struct pwm_output_t *pwm_output_p)
{
    assert(self_p != NULL);
    assert(name != NULL);
    assert(pwm_output_p != NULL);

    *self_p = (__typeof__(*self_p)) {0};
    self_p->pwm_output_p = pwm_output_p;
    self_p->logger_p = Logging_GetLogger(name);
    Logging_SetLevel(self_p->logger_p, MOTOR_LOGGER_DEBUG_LEVEL);

    SetupGPIO();
    SetupTimer();

    ResetPosition();

    PWM_Disable(self_p->pwm_output_p);
    PWM_SetFrequency(self_p->pwm_output_p, PWM_FREQUENCY);
    PWM_SetDuty(self_p->pwm_output_p, 0);

    /**
     * TODO: ADC for current consumption and input capture for encoder.
     */

    Logging_Info(self_p->logger_p, "Motor(%s) initialized", name);
}

int16_t Motor_GetRPM(const struct motor_t *self_p)
{
    assert(self_p != NULL);
    return 0;
}

int16_t Motor_GetCurrent(const struct motor_t *self_p)
{
    assert(self_p != NULL);
    return 0;
}

void Motor_SetSpeed(struct motor_t *self_p, int16_t speed)
{
    assert(self_p != NULL);
    assert(speed >= -100 && speed <= 100);

    self_p->direction = speed >= 0 ? MOTOR_DIR_CW : MOTOR_DIR_CCW;
    self_p->speed = speed;

    const uint16_t duty_cycle = SpeedToDutyCycle(speed);

    PWM_Disable(self_p->pwm_output_p);
    SetDirection(self_p->direction);
    PWM_SetDuty(self_p->pwm_output_p, duty_cycle);
    PWM_Enable(self_p->pwm_output_p);

    Logging_Debug(self_p->logger_p, "{speed: %i, duty: %u}", speed, duty_cycle);
}

void Motor_Run(struct motor_t *self_p)
{
    Motor_SetSpeed(self_p, self_p->speed);
}

void Motor_Coast(struct motor_t *self_p)
{
    assert(self_p != NULL);

    PWM_SetDuty(self_p->pwm_output_p, 0);

    Logging_Debug(self_p->logger_p, "Coasting enabled");
}

void Motor_Brake(struct motor_t *self_p)
{
    assert(self_p != NULL);

    PWM_Disable(self_p->pwm_output_p);

    SetGpio(GPIO_INA, false);
    SetGpio(GPIO_INB, false);

    const uint32_t max_duty = 100;
    PWM_SetDuty(self_p->pwm_output_p, max_duty);
    PWM_Enable(self_p->pwm_output_p);

    Logging_Debug(self_p->logger_p, "Braking enabled");
}

enum motor_status_t Motor_GetStatus(const struct motor_t *self_p)
{
    assert(self_p != NULL);
    return MOTOR_UNKNOWN;
}

enum motor_direction_t Motor_GetDirection(const struct motor_t *self_p)
{
    assert(self_p != NULL);
    return GetDIRValue() == 0 ? MOTOR_DIR_CW : MOTOR_DIR_CCW;
}

uint32_t Motor_GetPosition(const struct motor_t *self_p)
{
    assert(self_p != NULL);
    return (GetPosition() * 360) / 29;
}

const char *Motor_DirectionToString(const struct motor_t *self_p, enum motor_direction_t direction)
{
    assert(self_p != NULL);

    switch (direction)
    {
        case MOTOR_DIR_CW:
            return STRINGIFY(MOTOR_DIR_CW);
        case MOTOR_DIR_CCW:
            return STRINGIFY(MOTOR_DIR_CCW);
        default:
            Logging_Warning(self_p->logger_p, "Unknown direction: 0x%x", direction);
            return "UNKNOWN";
    }
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static inline void SetupGPIO(void)
{
    rcc_periph_clock_enable(RCC_GPIOC);

    const uint16_t gpios = GPIO_SEL | GPIO_INA | GPIO_INB;
    gpio_clear(GPIO_PORT, gpios);
    gpio_set_mode(GPIO_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, gpios);
    gpio_set_mode(GPIO_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO_CS);

    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO6);
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO7);
    gpio_clear(GPIOB, GPIO6 | GPIO7);
}

static inline void SetupTimer(void)
{
    const uint8_t encoder_mode = TIM_SMCR_SMS_EM1;
    const uint32_t counts_per_revolution = 29;

    rcc_periph_clock_enable(ENCODER_CLKEN);
    rcc_periph_reset_pulse(RST_TIM4);

    timer_set_period(ENCODER_TIMER_PERIPHERAL, counts_per_revolution);
    timer_slave_set_mode(ENCODER_TIMER_PERIPHERAL, encoder_mode);
    timer_ic_disable(ENCODER_TIMER_PERIPHERAL, TIM_IC1);
    timer_ic_disable(ENCODER_TIMER_PERIPHERAL, TIM_IC2);
    timer_ic_set_input(ENCODER_TIMER_PERIPHERAL, TIM_IC1, TIM_IC_IN_TI1);
    timer_ic_set_input(ENCODER_TIMER_PERIPHERAL, TIM_IC2, TIM_IC_IN_TI2);
    timer_ic_enable(ENCODER_TIMER_PERIPHERAL, TIM_IC1);
    timer_ic_enable(ENCODER_TIMER_PERIPHERAL, TIM_IC2);

    timer_enable_counter(ENCODER_TIMER_PERIPHERAL);
}

static inline uint16_t SpeedToDutyCycle(int16_t speed)
{
    uint16_t duty_cycle = (uint16_t)speed;
    if (speed < 0)
    {
        duty_cycle = (uint16_t)(speed * -1);
    }

    return duty_cycle;
}

static void SetGpio(uint16_t gpio, bool state)
{
    if (state)
    {
        gpio_set(GPIO_PORT, gpio);
    }
    else
    {
        gpio_clear(GPIO_PORT, gpio);
    }
}

static inline void SetDirection(enum motor_direction_t direction)
{
    if (direction == MOTOR_DIR_CW)
    {
        SetGpio(GPIO_INA, true);
        SetGpio(GPIO_INB, false);
        SetGpio(GPIO_SEL, true);
    }
    else
    {
        SetGpio(GPIO_INA, false);
        SetGpio(GPIO_INB, true);
        SetGpio(GPIO_SEL, false);
    }
}

static inline void ResetPosition(void)
{
    timer_set_counter(ENCODER_TIMER_PERIPHERAL, 0);
}

static inline uint32_t GetPosition(void)
{
    return timer_get_counter(ENCODER_TIMER_PERIPHERAL);
}

static inline uint32_t GetDIRValue(void)
{
    return (TIM_CR1(ENCODER_TIMER_PERIPHERAL) >> 4) & 1;
}
