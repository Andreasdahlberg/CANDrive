/**
 * @file   firmware_manager.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Firmware manager.
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

#ifndef FIRMWARE_MANAGER_H_
#define FIRMWARE_MANAGER_H_

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

typedef bool (*firmware_manager_allowed_t)(void);
typedef void (*firmware_manager_reset_t)(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize the firmware manager.
 */
void FirmwareManager_Init(firmware_manager_reset_t reset);

/**
 * Set callbacks used determine if an action is allowed.
 *
 * @param reset Callback for reset check, NULL if always allowed.
 * @param update Callback for update check, NULL if always allowed.
 */
void FirmwareManager_SetActionChecks(firmware_manager_allowed_t reset, firmware_manager_allowed_t update);

/**
 * Update the internal state.
 */
void FirmwareManager_Update(void);

/**
 * Check if the firmware manager is active.
 *
 * @return True if active, otherwise false.
 */
bool FirmwareManager_Active(void);

/**
 * Check if the firmware manager is downloading new firmware.
 *
 * @return True if downloading, otherwise false.
 */
bool FirmwareManager_DownloadActive(void);

#endif
