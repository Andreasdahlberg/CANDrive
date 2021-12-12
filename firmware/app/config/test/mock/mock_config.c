/**
 * @file   mock_config.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for config.
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
#include "config.h"

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

__attribute__((weak)) void Config_Init(void)
{
    function_called();
}

__attribute__((weak)) bool Config_IsValid(void)
{
    return mock_type(bool);
}

__attribute__((weak)) uint32_t Config_GetNumberOfMotors(void)
{
    return mock_type(uint32_t);
}

__attribute__((weak)) uint32_t Config_GetCountsPerRev(void)
{
    return mock_type(uint32_t);
}

__attribute__((weak)) uint32_t Config_GetNoLoadRpm(void)
{
    return mock_type(uint32_t);
}

__attribute__((weak)) uint32_t Config_GetNoLoadCurrent(void)
{
    return mock_type(uint32_t);
}

__attribute__((weak)) uint32_t Config_GetStallCurrent(void)
{
    return mock_type(uint32_t);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
