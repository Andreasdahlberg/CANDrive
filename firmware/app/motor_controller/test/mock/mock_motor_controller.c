/**
 * @file   mock_motor_controller.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for motor_controller.
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
#include "motor_controller.h"

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

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

__attribute__((weak)) void MotorController_Init(void)
{
    function_called();
}

__attribute__((weak)) void MotorController_Update(void)
{
    function_called();
}

__attribute__((weak)) void MotorController_SetRPM(size_t index, int16_t rpm)
{
    check_expected(index);
    check_expected(rpm);
}

__attribute__((weak)) void MotorController_SetCurrent(size_t index, int16_t current)
{
    check_expected(index);
    check_expected(current);
}

__attribute__((weak)) void MotorController_Run(size_t index)
{
    check_expected(index);
}

__attribute__((weak)) void MotorController_Coast(size_t index)
{
    check_expected(index);
}

__attribute__((weak)) void MotorController_Brake(size_t index)
{
    check_expected(index);
}

__attribute__((weak)) uint32_t MotorController_GetPosition(size_t index)
{
    return mock_type(uint32_t);
}

__attribute__((weak)) struct motor_controller_motor_status_t MotorController_GetStatus(size_t index)
{
    return *mock_ptr_type(struct motor_controller_motor_status_t *);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
