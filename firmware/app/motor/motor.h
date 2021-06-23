/**
 * @file   motor.h
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

#ifndef MOTOR_H_
#define MOTOR_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "logging.h"
#include "pwm.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct motor_t
{
    pwm_output_t *pwm_output_p;
    logging_logger_t *logger_p;
    int16_t speed;
    bool forward;
};

enum motor_status_t
{
    MOTOR_OK = 0,
    MOTOR_UNKNOWN,
    MOTOR_SHORT_TO_GROUND,
    MOTOR_SHORT_TO_VCC,
    MOTOR_OPEN_LOAD,
    MOTOR_THERMAL_SHUTDOWN
};

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize the motor instance.
 *
 * @param self_p Pointer to motor instance.
 * @param name Name of the motor instance, used for logging.
 * @param pwm_p Pointer to PWM output instance(must be initialized).
 */
void Motor_Init(struct motor_t *self_p, char *name, struct pwm_output_t *pwm_p);

/**
 * Get the motor RPM.
 *
 * @param self_p Pointer to motor instance.
 *
 * @return Motor RPM.
 */
int16_t Motor_GetRPM(const struct motor_t *self_p);

/**
 * Get the current consumption.
 *
 * @param self_p Pointer to motor instance.
 *
 * @return Motor current in mA.
 */
int16_t Motor_GetCurrent(const struct motor_t *self_p);

/**
 * Set the motor speed.
 *
 * @param self_p Pointer to motor instance.
 * @param speed  Motor speed, direction depends on the speed sign. [-100, 100]
 */
void Motor_SetSpeed(struct motor_t *self_p, int16_t speed);

/**
 * Resume running after coast/brake.
 *
 * The speed set before coast/brake will be used.
 *
 * @param self_p Pointer to motor instance.
 */
void Motor_Run(struct motor_t *self_p);

/**
 * Stop the motor without braking.
 *
 * @param self_p Pointer to motor instance.
 */
void Motor_Coast(struct motor_t *self_p);

/**
 * Stop the motor and brake.
 *
 * @param self_p Pointer to motor instance.
 */
void Motor_Brake(struct motor_t *self_p);

enum motor_status_t Motor_GetStatus(struct motor_t *self_p);

#endif
