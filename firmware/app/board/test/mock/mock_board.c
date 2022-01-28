/**
 * @file   mock_board.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for board.
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
#include "board.h"

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

__attribute__((weak)) void Board_Init(void)
{
    function_called();
}

__attribute__((weak)) uint32_t Board_GetHardwareRevision(void)
{
    return mock_type(uint32_t);
}

__attribute__((weak)) uint32_t Board_GetSoftwareRevision(void)
{
    return mock_type(uint32_t);
}

__attribute__((weak)) struct board_id_t Board_GetId(void)
{
    struct board_id_t id = {.offset_0 = 1, .offset_4 = 2, .offset_8 = 3};
    return id;
}

__attribute__((weak)) const struct motor_config_t *Board_GetMotorConfig(size_t index)
{
    return mock_type(struct motor_config_t *);
}

__attribute__((weak)) size_t Board_GetMaxNumberOfMotors(void)
{
    return mock_type(size_t);
}

__attribute__((weak)) void Board_ToggleStatusLED(void)
{
    function_called();
}

__attribute__((weak)) uint32_t Board_GetResetFlags(void)
{
    return mock_type(uint32_t);
}

__attribute__((weak)) void Board_Reset(void)
{
    function_called();
}

__attribute__((weak)) bool Board_GetEmergencyPinState(void)
{
    return mock_type(bool);
}

__attribute__((weak)) uintptr_t Board_GetNVSAddress(void)
{
    return mock_type(uintptr_t);
}

__attribute__((weak)) uintptr_t Board_GetUpgradeMemoryAddress(void)
{
    return mock_type(uintptr_t);
}

__attribute__((weak)) uint32_t Board_GetNumberOfPagesInNVS(void)
{
    return mock_type(uint32_t);
}

__attribute__((weak)) uint32_t Board_GetMaxCurrent(void)
{
    return mock_type(uint32_t);
}

__attribute__((weak)) uintptr_t Board_GetBackupMemoryAddress(void)
{
    return mock_type(uintptr_t);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
