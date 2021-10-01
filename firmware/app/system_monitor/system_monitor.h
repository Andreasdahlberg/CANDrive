/**
 * @file   system_monitor.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  System monitor module.
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

#ifndef SYSTEM_MONITOR_H_
#define SYSTEM_MONITOR_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>

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
 * Initialize the system monitor.
 */
void SystemMonitor_Init(void);

void SystemMonitor_Update(void);


uint32_t SystemMonitor_GetWatchdogHandle(void);


void SystemMonitor_FeedWatchdog(uint32_t handle);

#endif
