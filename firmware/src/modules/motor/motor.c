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
#include <stdlib.h>
#include "utility.h"
#include "systime.h"
#include "config.h"
#include "motor.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#ifndef MOTOR_LOGGER_DEBUG_LEVEL
#define MOTOR_LOGGER_DEBUG_LEVEL LOGGING_INFO
#endif

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static const uint32_t PWM_FREQUENCY = 20000;
static const uint32_t RPM_SAMPLE_FREQUENCY = 100;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static inline void SetupGPIO(const struct motor_t *self_p);
static inline void SetupTimer(const struct motor_t *self_p);
static inline uint16_t SpeedToDutyCycle(int16_t speed);
static inline void SetGpio(const struct motor_t *self_p, uint16_t gpio, bool state);
static inline void SetDirection(const struct motor_t *self_p);
static inline uint32_t GetPosition(const struct motor_t *self_p);
static inline void ResetPosition(const struct motor_t *self_p);
static inline int16_t SenseVoltageToCurrent(const struct motor_t *self_p, uint32_t sense_voltage);
static inline int32_t GetCountDifference(const struct motor_t *self_p, int32_t count);
static inline int32_t CountToRPM(const struct motor_t *self_p, int32_t count, uint32_t frequency);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void Motor_Init(struct motor_t *self_p,
                const char *name,
                const struct motor_config_t *config_p)
{
    assert(self_p != NULL);
    assert(name != NULL);
    assert(config_p != NULL);

    *self_p = (__typeof__(*self_p)) {0};
    self_p->config_p = config_p;
    self_p->status = MOTOR_RUN;
    self_p->counts_per_revolution = (int32_t)Config_GetCountsPerRev();
    assert(self_p->counts_per_revolution > 0);

    self_p->logger_p = Logging_GetLogger(name);
    Logging_SetLevel(self_p->logger_p, MOTOR_LOGGER_DEBUG_LEVEL);

    SetupGPIO(self_p);
    SetupTimer(self_p);

    ResetPosition(self_p);

    ADC_InitChannel(&self_p->adc_input, self_p->config_p->adc.channel);

    PWM_Init(&self_p->pwm_output, &config_p->pwm);
    PWM_Disable(&self_p->pwm_output);
    PWM_SetFrequency(&self_p->pwm_output, PWM_FREQUENCY);
    PWM_SetDuty(&self_p->pwm_output, 0);

    Logging_Info(self_p->logger_p, "Motor(%s) initialized", name);
}

void Motor_Update(struct motor_t *self_p)
{
    assert(self_p != NULL);
    const uint32_t update_period_ms = 1000 / RPM_SAMPLE_FREQUENCY;

    const uint32_t time_since_last_update = SysTime_GetDifference(self_p->timer);
    if (time_since_last_update >= update_period_ms)
    {
        const int32_t count = (int32_t)GetPosition(self_p);
        const int32_t difference = GetCountDifference(self_p, count);
        const uint32_t actual_sample_frequency = 1000 / time_since_last_update;

        self_p->rpm = (int16_t)CountToRPM(self_p, difference, actual_sample_frequency);
        self_p->count = count;
        self_p->timer = SysTime_GetSystemTime();
    }
}

int16_t Motor_GetRPM(const struct motor_t *self_p)
{
    assert(self_p != NULL);
    return self_p->rpm;
}

int16_t Motor_GetCurrent(const struct motor_t *self_p)
{
    assert(self_p != NULL);

    const uint32_t current_sense_voltage = ADC_GetVoltage(&self_p->adc_input);
    return SenseVoltageToCurrent(self_p, current_sense_voltage);
}

void Motor_SetSpeed(struct motor_t *self_p, int16_t speed)
{
    assert(self_p != NULL);
    assert(speed >= -1000 && speed <= 1000);

    if ((self_p->status != MOTOR_RUN) || (speed != self_p->speed))
    {
        self_p->speed = speed;
        const uint16_t duty_cycle = SpeedToDutyCycle(speed);

        PWM_Disable(&self_p->pwm_output);
        SetDirection(self_p);
        PWM_SetDuty(&self_p->pwm_output, duty_cycle);
        PWM_Enable(&self_p->pwm_output);

        self_p->status = MOTOR_RUN;
        Logging_Debug(self_p->logger_p, "{rpm: % i, speed: %i}", self_p->rpm, speed);
    }
}

void Motor_Coast(struct motor_t *self_p)
{
    assert(self_p != NULL);

    PWM_SetDuty(&self_p->pwm_output, 0);
    self_p->status = MOTOR_COAST;

    Logging_Info(self_p->logger_p, "Coasting enabled");
}

void Motor_Brake(struct motor_t *self_p)
{
    assert(self_p != NULL);

    PWM_Disable(&self_p->pwm_output);

    SetGpio(self_p, self_p->config_p->driver.ina, false);
    SetGpio(self_p, self_p->config_p->driver.inb, false);

    const uint32_t max_duty = 1000;
    PWM_SetDuty(&self_p->pwm_output, max_duty);
    PWM_Enable(&self_p->pwm_output);
    self_p->status = MOTOR_BRAKE;

    Logging_Info(self_p->logger_p, "Braking enabled");
}

enum motor_status_t Motor_GetStatus(const struct motor_t *self_p)
{
    assert(self_p != NULL);
    return self_p->status;
}

enum motor_direction_t Motor_GetDirection(const struct motor_t *self_p)
{
    assert(self_p != NULL);

    return timer_get_direction(self_p->config_p->encoder.timer) == 0 ? MOTOR_DIR_CW : MOTOR_DIR_CCW;
}

uint32_t Motor_GetPosition(const struct motor_t *self_p)
{
    assert(self_p != NULL);
    return (GetPosition(self_p) * 360) / (uint32_t)self_p->counts_per_revolution;
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

static inline void SetupGPIO(const struct motor_t *self_p)
{
    const uint16_t gpios = self_p->config_p->driver.sel | self_p->config_p->driver.ina | self_p->config_p->driver.inb;
    rcc_periph_clock_enable(self_p->config_p->driver.gpio_clock);
    gpio_clear(self_p->config_p->driver.port, gpios);
    gpio_set_mode(self_p->config_p->driver.port, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, gpios);
    gpio_set_mode(self_p->config_p->driver.port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, self_p->config_p->driver.cs);

    rcc_periph_clock_enable(self_p->config_p->encoder.gpio_clock);
    gpio_set_mode(self_p->config_p->encoder.port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, self_p->config_p->encoder.a);
    gpio_set_mode(self_p->config_p->encoder.port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, self_p->config_p->encoder.b);
    gpio_clear(self_p->config_p->encoder.port, self_p->config_p->encoder.a | self_p->config_p->encoder.b);
}

static inline void SetupTimer(const struct motor_t *self_p)
{
    const uint8_t encoder_mode = TIM_SMCR_SMS_EM3;

    rcc_periph_clock_enable(self_p->config_p->encoder.timer_clock);
    rcc_periph_reset_pulse(self_p->config_p->encoder.timer_rst);

    timer_set_period(self_p->config_p->encoder.timer, (uint32_t)self_p->counts_per_revolution - 1);
    timer_slave_set_mode(self_p->config_p->encoder.timer, encoder_mode);
    timer_ic_disable(self_p->config_p->encoder.timer, TIM_IC1);
    timer_ic_disable(self_p->config_p->encoder.timer, TIM_IC2);
    timer_ic_set_input(self_p->config_p->encoder.timer, TIM_IC1, TIM_IC_IN_TI1);
    timer_ic_set_input(self_p->config_p->encoder.timer, TIM_IC2, TIM_IC_IN_TI2);
    timer_ic_set_filter(self_p->config_p->encoder.timer, TIM_IC1, TIM_IC_CK_INT_N_8);
    timer_ic_set_filter(self_p->config_p->encoder.timer, TIM_IC2, TIM_IC_CK_INT_N_8);
    timer_ic_enable(self_p->config_p->encoder.timer, TIM_IC1);
    timer_ic_enable(self_p->config_p->encoder.timer, TIM_IC2);
    timer_enable_counter(self_p->config_p->encoder.timer);
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

static inline void SetGpio(const struct motor_t *self_p, uint16_t gpio, bool state)
{
    if (state)
    {
        gpio_set(self_p->config_p->driver.port, gpio);
    }
    else
    {
        gpio_clear(self_p->config_p->driver.port, gpio);
    }
}

static inline void SetDirection(const struct motor_t *self_p)
{
    if (self_p->speed > 0)
    {
        SetGpio(self_p, self_p->config_p->driver.ina, true);
        SetGpio(self_p, self_p->config_p->driver.inb, false);
        SetGpio(self_p, self_p->config_p->driver.sel, true);
    }
    else if (self_p->speed < 0)
    {
        SetGpio(self_p, self_p->config_p->driver.ina, false);
        SetGpio(self_p, self_p->config_p->driver.inb, true);
        SetGpio(self_p, self_p->config_p->driver.sel, false);
    }
    else
    {
        /*Do not update the direction when the speed is set to 0.*/
    }
}

static inline void ResetPosition(const struct motor_t *self_p)
{
    timer_set_counter(self_p->config_p->encoder.timer, 0);
}

static inline uint32_t GetPosition(const struct motor_t *self_p)
{
    return timer_get_counter(self_p->config_p->encoder.timer);
}

static inline int16_t SenseVoltageToCurrent(const struct motor_t *self_p, uint32_t sense_voltage)
{
    assert(sense_voltage < INT16_MAX);

    int16_t current = (int16_t)sense_voltage;

    if (Motor_GetDirection(self_p) == MOTOR_DIR_CCW)
    {
        current *= -1;
    }

    return current;
}

static inline int32_t GetCountDifference(const struct motor_t *self_p, int32_t count)
{
    int32_t difference = count - self_p->count;
    const bool wrap_around = abs(difference) > (self_p->counts_per_revolution / 2);
    if (wrap_around)
    {
        if (difference > 0)
        {
            difference = difference - self_p->counts_per_revolution - 1;
        }
        else
        {
            difference = self_p->counts_per_revolution + difference + 1;
        }
    }
    return difference;
}

static inline int32_t CountToRPM(const struct motor_t *self_p, int32_t count, uint32_t frequency)
{
    return (count * (int32_t)frequency * 60 + (self_p->counts_per_revolution / 2)) / self_p->counts_per_revolution;
}
