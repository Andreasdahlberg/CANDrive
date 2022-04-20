/**
 * @file   board.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Board support module.
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

#ifndef BOARD_H_
#define BOARD_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include "motor.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define BOARD_M1_INDEX 0
#define BOARD_M2_INDEX 1

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct board_id_t
{
    uint32_t offset_0;
    uint32_t offset_4;
    uint32_t offset_8;
};

struct firmware_information_t
{
    uint32_t version;
    uint32_t length;
    char name[12];
};

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize the board.
 */
void Board_Init(void);

/**
 * Get the hardware revision.
 *
 * @return Hardware revision.
 */
uint32_t Board_GetHardwareRevision(void);

/**
 * Get the software revision.
 *
 * @return software revision.
 */
uint32_t Board_GetSoftwareRevision(void);

/**
 * Get the device id.
 *
 * @return 96-bit device id.
 */
struct board_id_t Board_GetId(void);

/**
 * Get motor configuration.
 *
 * @param index Index of motor.
 * @return Pointer to motor configuration.
 */
const struct motor_config_t *Board_GetMotorConfig(size_t index);

/**
 * Get the max number of motors that can be connected.
 *
 * @return Max number of motors.
 */
size_t Board_GetMaxNumberOfMotors(void);

/**
 * Toggle the status LED.
 */
void Board_ToggleStatusLED(void);

/**
 * Get the MCU reset flags.
 *
 * @return Reset flags.
 */
uint32_t Board_GetResetFlags(void);

/**
 * Reset the device.
 */
void Board_Reset(void);

/**
 * Get the state of the emergency pin.
 *
 * @return Emergency pin state.
 */
bool Board_GetEmergencyPinState(void);

/**
 * Get the start address of the NVS partition.
 *
 * @return Address.
 */
uintptr_t Board_GetNVSAddress(void);

/**
 * Get the number of flash pages in the NVS partition.
 *
 * @return Number of pages.
 */
uint32_t Board_GetNumberOfPagesInNVS(void);

/**
 * Get the start address of the application partition.
 *
 * @return Address.
 */
uintptr_t Board_GetApplicationAddress(void);

/**
 * Get the max current that the board can deliver.
 *
 * @return Max current in mA.
 */
uint32_t Board_GetMaxCurrent(void);

/**
 * Get the backup memory address.
 *
 * @return Address.
 */
uintptr_t Board_GetBackupMemoryAddress(void);

#endif