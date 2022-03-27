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

enum system_monitor_state_t
{
    SYSTEM_MONITOR_UNKNOWN = 0,
    SYSTEM_MONITOR_ACTIVE,
    SYSTEM_MONITOR_FAIL,
    SYSTEM_MONITOR_INACTIVE,
    SYSTEM_MONITOR_EMERGENCY,
    /* No new states after SYSTEM_MONITOR_END!*/
    SYSTEM_MONITOR_END
};

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize the system monitor.
 */
void SystemMonitor_Init(void);

/**
 * Update the internal state of the system monitor.
 */
void SystemMonitor_Update(void);

/**
 * Get a unique watchdog handle.
 *
 * After calling this function, 'SystemMonitor_FeedWatchdog' must by called with
 * the handle as input to prevent the watchdog from resetting the device.
 *
 * @return Watchdog handle.
 */
uint32_t SystemMonitor_GetWatchdogHandle(void);

/**
 * Feed the watchdog associated with the supplied handle.
 *
 * @param handle
 */
void SystemMonitor_FeedWatchdog(uint32_t handle);

/**
 * Report control activity.
 */
void SystemMonitor_ReportActivity(void);

enum system_monitor_state_t SystemMonitor_GetState(void);

#endif
