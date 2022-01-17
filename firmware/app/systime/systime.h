/**
 * @file   systime.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  System time module.
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

#ifndef SYSTIME_H_
#define SYSTIME_H_

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
 * Initialize the system time module.
 */
void SysTime_Init(void);

/**
 * Get the number of milliseconds since system start.
 *
 * Wraps around in ~50 days.
 *
 * @return Timestamp in milliseconds.
 */
uint32_t SysTime_GetSystemTime(void);

/**
 * Get the number of microseconds since system start.
 *
 * Wraps around in ~70 minutes.
 *
 * @return Timestamp in microseconds.
 */
uint32_t SysTime_GetSystemTimeUs(void);

/**
 * Get the number of seconds since system start.
 *
 * @return Timestamp in seconds.
 */
uint32_t SysTime_GetSystemTimestamp(void);

/**
 * Get the difference between the current system time and the supplied system time.
 *
 * Protected against timer wrap-around. CAUTION: Does not detect several
 * wrap-arounds but this is a very long time(over 49 days).
 *
 * @param system_time System time in ms.
 *
 * @return Difference in ms.
 */
uint32_t SysTime_GetDifference(uint32_t system_time);

#endif
