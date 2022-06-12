/**
 * @file   mock_system_monitor.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for system_monitor.
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
#include "system_monitor.h"

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

__attribute__((weak)) void SystemMonitor_Init(void)
{
    function_called();
}

__attribute__((weak)) void SystemMonitor_Update(void)
{
    function_called();
}

__attribute__((weak)) uint32_t SystemMonitor_GetWatchdogHandle(void)
{
    return mock_type(uint32_t);
}

__attribute__((weak)) void SystemMonitor_FeedWatchdog(uint32_t handle)
{
    check_expected(handle);
}

__attribute__((weak)) void SystemMonitor_ReportActivity(void)
{
    function_called();
}

__attribute__((weak)) enum system_monitor_state_t SystemMonitor_GetState(void)
{
    return mock_type(enum system_monitor_state_t);
}

__attribute__((weak)) uint32_t SystemMonitor_GetResetFlags(void)
{
    return mock_type(uint32_t);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
