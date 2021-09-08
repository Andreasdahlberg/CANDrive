/**
 * @file   motor_manager.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Motor manager module.
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

#ifndef MOTOR_CONTROLLER_H_
#define MOTOR_CONTROLLER_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "motor.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize the motor controller.
 *
 * All associated drivers and motors will be initialized.
 */
void MotorController_Init(void);

/**
 * Update the internal state of the motor controller.
 *
 * Call as fast as possible.
 */
void MotorController_Update(void);

/**
 * Set the target RPM for the selected motor.
 * @param index Motor index.
 * @param rpm Target RPM:
 */
void MotorController_SetRPM(size_t index, int16_t rpm);

/**
 * Set the target CURRENT for the selected motor.
 * @param index Motor index.
 * @param current Target CURRENT:
 */
void MotorController_SetCurrent(size_t index, int16_t current);

/**
 * Stop the selected motor without braking.
 *
 * @param index Motor index.
 */
void MotorController_Coast(size_t index);

/**
 * Stop the selected motor and brake.
 *
 * @param index Motor index.
 */
void MotorController_Brake(size_t index);

/**
 * Get the position of the selected motor.
 *
 * @param index Motor index.
 *
 * @return Motor position in degrees.
 */
uint32_t MotorController_GetPosition(size_t index);

#endif
