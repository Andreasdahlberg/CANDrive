/**
 * @file   motor_controller_cmd.c
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

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdio.h>
#include "utility.h"
#include "console.h"
#include "board.h"
#include "motor_controller.h"
#include "motor_controller_cmd.h"

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

static bool GetIndex(size_t *index_p);
static bool GetRPM(int16_t *rpm_p);
static bool GetCurrent(int16_t *current_p);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

bool MotorControllerCmd_SetRPM(void)
{
    bool status = false;

    size_t index;
    int16_t rpm;
    if (GetIndex(&index) && GetRPM(&rpm))
    {
        MotorController_SetRPM(index, rpm);
        status = true;
    }
    return status;
}

bool MotorControllerCmd_SetCurrent(void)
{
    bool status = false;

    size_t index;
    int16_t current;
    if (GetIndex(&index) && GetCurrent(&current))
    {
        MotorController_SetCurrent(index, current);
        status = true;
    }
    return status;
}

bool MotorControllerCmd_Coast(void)
{
    bool status = false;

    size_t index;
    if (GetIndex(&index))
    {
        MotorController_Coast(index);
        status = true;
    }
    return status;
}

bool MotorControllerCmd_Brake(void)
{
    bool status = false;

    size_t index;
    if (GetIndex(&index))
    {
        MotorController_Brake(index);
        status = true;
    }
    return status;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static bool GetIndex(size_t *index_p)
{
    bool status = false;

    int32_t arg;
    if (Console_GetArgument(&arg))
    {
        if (arg >= 0 && (size_t)arg < Board_GetNumberOfMotors())
        {
            *index_p = (size_t)arg;
            status = true;
        }
    }

    return status;
}

static bool GetRPM(int16_t *rpm_p)
{
    bool status = false;

    int32_t arg;
    if (Console_GetArgument(&arg))
    {
        if (arg >= INT16_MIN && arg <= INT16_MAX)
        {
            *rpm_p = (int16_t)arg;
            status = true;
        }
    }

    return status;
}

static bool GetCurrent(int16_t *current_p)
{
    bool status = false;

    int32_t arg;
    if (Console_GetArgument(&arg))
    {
        if (arg >= INT16_MIN && arg <= INT16_MAX)
        {
            *current_p = (int16_t)arg;
            status = true;
        }
    }

    return status;
}
