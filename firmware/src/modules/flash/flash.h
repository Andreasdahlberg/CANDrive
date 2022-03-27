/**
 * @file   flash.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Flash helper functions.
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

#ifndef FLASH_H_
#define FLASH_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

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
 * Initialize the flash helper.
 */
void Flash_Init(void);

/**
 * Write data to flash.
 *
 * Note: Pages must be erased before written.
 *
 * @param address Destination address.
 * @param data_p Pointer to data source.
 * @param length Number of bytes to write.
 *
 * @return True if write was successful, otherwise false.
 */
bool Flash_Write(uint32_t address, const void *data_p, size_t length);

/**
 * Erase page in flash.
 *
 * @param page_address Address of page to erase.
 *
 * @return True if erase was successful, otherwise false.
 */
bool Flash_ErasePage(uint32_t page_address);

#endif
