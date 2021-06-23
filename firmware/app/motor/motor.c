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
#include <assert.h>
#include <math.h>
#include "utility.h"
#include "motor.h"

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

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static inline void SetupGPIO(void);
static inline uint16_t SpeedToDutyCycle(int16_t speed);
static inline void SetGpio(uint16_t gpio, bool state);
static inline void SetDirection(bool forward);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void Motor_Init(struct motor_t *self_p, char *name, struct pwm_output_t *pwm_output_p)
{
    assert(self_p != NULL);
    assert(pwm_output_p != NULL);

    *self_p = (__typeof__(*self_p)) {0};
    self_p->pwm_output_p = pwm_output_p;
    self_p->logger_p = Logging_GetLogger(name);
    Logging_SetLevel(self_p->logger_p, MOTOR_LOGGER_DEBUG_LEVEL);

    SetupGPIO();

    PWM_Disable(self_p->pwm_output_p);
    PWM_SetFrequency(self_p->pwm_output_p, PWM_FREQUENCY);
    PWM_SetDuty(self_p->pwm_output_p, 0);

    /**
     * TODO: ADC for current consumption, Input capture for encoder and
     *       pins for direction.
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

    self_p->forward = (bool)(speed >= 0);
    self_p->speed = speed;

    const uint16_t duty_cycle = SpeedToDutyCycle(speed);

    PWM_Disable(self_p->pwm_output_p);
    SetDirection(self_p->forward);
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

enum motor_status_t Motor_GetStatus(struct motor_t *self_p)
{
    assert(self_p != NULL);
    return MOTOR_UNKNOWN;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static inline void SetupGPIO(void)
{
    rcc_periph_clock_enable(RCC_GPIOC);

    const uint16_t gpios = GPIO_SEL | GPIO_INA | GPIO_INB;
    gpio_set(GPIO_PORT, gpios);
    gpio_set_mode(GPIO_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, gpios);
    gpio_set_mode(GPIO_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO_CS);
}

static inline uint16_t SpeedToDutyCycle(int16_t speed)
{
    uint16_t duty_cycle = speed;
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

static inline void SetDirection(bool forward)
{
    SetGpio(GPIO_INA, forward);
    SetGpio(GPIO_INB, !forward);
    SetGpio(GPIO_SEL, forward);
}
