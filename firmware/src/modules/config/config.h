/**
 * @file   config.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Implementation of device configuration module.
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

#include <stdint.h>
#include <stdbool.h>

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
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize the configuration.
 *
 * All parameters are retrieved from NVS into RAM.
 */
void Config_Init(void);

/**
 * Check if the configurations is valid.
 *
 * @return True if all parameters was set and read from NVS, otherwise false.
 */
bool Config_IsValid(void);

/**
 * Get a value by name.
 *
 * @param  name_p Name of value.
 *
 * @return Value.
 */
uint32_t Config_GetValue(const char *name_p);

/**
 * Get the number of connected motors.
 *
 * @return Number of connected motors.
 */
uint32_t Config_GetNumberOfMotors(void);

/**
 * Get the number of counts per revolution on the motor rotary encoder.
 *
 * @return Number of counts per revolution.
 */
uint32_t Config_GetCountsPerRev(void);

/**
 * Get the no load RPM for the connected motors.
 *
 * @return No load RPM.
 */
uint32_t Config_GetNoLoadRpm(void);

/**
 * Get the no load current for the connected motors.
 *
 * @return No load current in mA.
 */
uint32_t Config_GetNoLoadCurrent(void);

/**
 * Get the stall current for the connected motors.
 *
 * @return Stall current in mA.
 */
uint32_t Config_GetStallCurrent(void);
