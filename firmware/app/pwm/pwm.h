/**
 * @file   pwm.h
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

#ifndef PWM_H_
#define PWM_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <stdint.h>
#include <stddef.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct pwm_config_t
{
    uint32_t timer_peripheral;
    uint32_t remap;
    uint32_t gpio_port;
    uint16_t gpio;
    enum tim_oc_id oc_id;
    enum rcc_periph_clken peripheral_clocks[3];
};

struct pwm_output_t
{
    const struct pwm_config_t *config_p;
    uint32_t frequency;
    uint32_t duty;
    uint32_t period;
};

typedef struct pwm_output_t pwm_output_t;

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize a PWM instance.
 *
 * @param self_p Pointer to PWM instance.
 * @param config_p Pointer to PWM configuration struct.
 */
void PWM_Init(pwm_output_t *self_p, const struct pwm_config_t *config_p);

/**
 * Set frequency on the PWM output.
 *
 * It's not recommended to change the frequency while the PWM output is enabled
 * since the duty cycle will be incorrect until a new call to 'PWM_SetDuty()' performed.
 *
 * @param self_p Pointer to PWM instance.
 * @param frequency Frequency in Hz.
 */
void PWM_SetFrequency(pwm_output_t *self_p, uint32_t frequency);

/**
 * Set duty cycle on the PWM output.
 *
 * @param self_p Pointer to PWM instance.
 * @param duty  Duty cycle in percent[0-100].
 */
void PWM_SetDuty(pwm_output_t *self_p, uint32_t duty);

/**
 * Enable the PWM output.
 *
 * @param self_p Pointer to PWM instance.
 */
void PWM_Enable(const pwm_output_t *self_p);

/**
 * Disable the PWM output.
 *
 * @param self_p Pointer to PWM instance.
 */
void PWM_Disable(const pwm_output_t *self_p);

#endif
