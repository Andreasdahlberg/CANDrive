/**
 * @file   pid.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  PID-controller module.
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

#ifndef PID_H_
#define PID_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct pid_parameters_t
{
    int32_t kp;
    int32_t ki;
    int32_t kd;
    int32_t imax;
    int32_t cvmax;
    int32_t scale;
};

struct pid_t
{
    int32_t last_error;
    int32_t last_integral;
    int32_t cv;
    int32_t sp;
    struct pid_parameters_t parameters;
};

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize a PID controller.
 *
 * @param self_p Pointer to a PID controller instance.
 */
void PID_Init(struct pid_t *self_p);

/**
 * Process a new input value and update the output value.
 *
 * @param self_p Pointer to a PID controller instance.
 * @param  input  Input value(pv).
 *
 * @return New output value(cv).
 */
int32_t PID_Update(struct pid_t *self_p, int32_t input);

/**
 * Set the set point for the PID controller.
 *
 * @param self_p Pointer to a PID controller instance.
 * @param setpoint Set point(sp).
 */
void PID_SetSetpoint(struct pid_t *self_p, int32_t setpoint);

/**
 * Get the set point for the PID controller.
 *
 * @param self_p Pointer to a PID controller instance.
 *
 * @return Set point.
 */
int32_t PID_GetSetpoint(const struct pid_t *self_p);

/**
 * Set the parameters for the PID controller.
 *
 * @param self_p Pointer to a PID controller instance.
 * @param parameters_p Pointer to parameter struct.
 */
void PID_SetParameters(struct pid_t *self_p, const struct pid_parameters_t *parameters_p);

/**
 * Get the parameters for the PID controller.
 *
 * @param self_p Pointer to a PID controller instance.
 *
 * @return Pointer to parameter struct.
 */
struct pid_parameters_t *PID_GetParameters(struct pid_t *self_p);

/**
 * Get the output value of the PID controller.
 *
 * @param self_p Pointer to a PID controller instance.
 *
 * @return Output value(cv).
 */
int32_t PID_GetOutput(const struct pid_t *self_p);

/**
 * Reset the PID controller.
 *
 * @param self_p Pointer to a PID controller instance.
 */
void PID_Reset(struct pid_t *self_p);

#endif
