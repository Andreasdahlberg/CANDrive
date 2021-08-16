/**
 * @file   pwm.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  PWM module.
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

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>

#include <assert.h>
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

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static inline void EnablePeripheralClocks(const struct pwm_config_t *config_p);
static inline void SetupGPIO(const struct pwm_config_t *config_p);
static inline void SetupTimer(const struct pwm_config_t *config_p);
static inline uint32_t DutyToOutputCompareValue(const pwm_output_t *self_p);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void PWM_Init(pwm_output_t *self_p, const struct pwm_config_t *config_p)
{
    assert(self_p != NULL);
    assert(config_p != NULL);

    self_p->config_p = config_p;

    EnablePeripheralClocks(self_p->config_p);
    SetupGPIO(self_p->config_p);
    SetupTimer(self_p->config_p);
}

void PWM_SetFrequency(pwm_output_t *self_p, uint32_t frequency)
{
    assert(self_p != NULL);
    assert(frequency != 0);

    self_p->frequency = frequency;

    const uint32_t main_frequency = 72000000;
    assert(frequency <= main_frequency);
    self_p->period = main_frequency / frequency;
    timer_set_period(self_p->config_p->timer_peripheral, self_p->period);
}

void PWM_SetDuty(pwm_output_t *self_p, uint32_t duty)
{
    assert(self_p != NULL);
    assert(duty <= 100);

    self_p->duty = duty;
    const uint32_t oc_compare_value = DutyToOutputCompareValue(self_p);
    timer_set_oc_value(self_p->config_p->timer_peripheral, self_p->config_p->oc_id, oc_compare_value);
}

void PWM_Enable(const pwm_output_t *self_p)
{
    assert(self_p != NULL);

    timer_enable_oc_output(self_p->config_p->timer_peripheral, self_p->config_p->oc_id);
    timer_enable_counter(self_p->config_p->timer_peripheral);
}

void PWM_Disable(const pwm_output_t *self_p)
{
    assert(self_p != NULL);

    timer_disable_counter(self_p->config_p->timer_peripheral);
    timer_disable_oc_output(self_p->config_p->timer_peripheral, self_p->config_p->oc_id);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static void EnablePeripheralClocks(const struct pwm_config_t *config_p)
{
    assert(config_p != NULL);

    for (size_t i = 0; i < ElementsIn(config_p->peripheral_clocks); ++i)
    {
        rcc_periph_clock_enable(config_p->peripheral_clocks[i]);
    }
}

static void SetupGPIO(const struct pwm_config_t *config_p)
{
    assert(config_p != NULL);

    gpio_set_mode(config_p->gpio_port,
                  GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                  config_p->gpio);

    gpio_primary_remap(config_p->remap, 0);
}

static void SetupTimer(const struct pwm_config_t *config_p)
{
    assert(config_p != NULL);

    timer_set_mode(config_p->timer_peripheral, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    timer_set_prescaler(config_p->timer_peripheral, 0);
    timer_set_repetition_counter(config_p->timer_peripheral, 0);
    timer_enable_preload(config_p->timer_peripheral);
    timer_continuous_mode(config_p->timer_peripheral);
    timer_set_oc_mode(config_p->timer_peripheral, config_p->oc_id, TIM_OCM_PWM1);
}

static inline uint32_t DutyToOutputCompareValue(const pwm_output_t *self_p)
{
    const uint32_t scaling_factor = 100;
    return ((self_p->period * self_p->duty) + (scaling_factor / 2)) / scaling_factor;
}
