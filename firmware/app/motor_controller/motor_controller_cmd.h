/**
 * @file   motor_controller_cmd.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Console commands for the motor controller module.
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

#ifndef MOTOR_CONTROLLER_CMD_H_
#define MOTOR_CONTROLLER_CMD_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdbool.h>

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
 * Set target RPM.
 *
 * @return Command status.
 */
bool MotorControllerCmd_SetRPM(void);

/**
 * Set target current.
 *
 * @return Command status.
 */
bool MotorControllerCmd_SetCurrent(void);

/**
 * Run motor after brake/coast.
 *
 * @return Command status.
 */
bool MotorControllerCmd_Run(void);

/**
 * Coast motor.
 *
 * @return Command status.
 */
bool MotorControllerCmd_Coast(void);

/**
 * Brake motor.
 *
 * @return Command status.
 */
bool MotorControllerCmd_Brake(void);

#endif
