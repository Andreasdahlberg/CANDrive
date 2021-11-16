/**
 * @file   nvs.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Non Volatile Storage module.
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

#ifndef NVS_H_
#define NVS_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize the Non Volatile Storage(NVS) module.
 *
 * @param start_page_address Address of the first flash page used for NVS.
 * @param number_of_pages Number of flash pages used for NVS.
 */
void NVS_Init(uint32_t start_page_address, size_t number_of_pages);

/**
 * Store a value in NVS.
 *
 * @param  key_p Key of value to store.
 * @param  value Value to store.
 * @return True if value was stored successfully, otherwise false.
 */
bool NVS_Store(const char *key_p, uint32_t value);

/**
 * Retrieve a value from NVS.
 *
 * @param  key_p Key of value to retrieve.
 * @param  value_p Pointer to the retrieved value.
 *
 * @return True if value for the supplied key existed, otherwise false.
 */
bool NVS_Retrieve(const char *key_p, uint32_t *value_p);

/**
 * Clear all stored values.
 *
 * @return Status of clear operation.
 */
bool NVS_Clear(void);

#endif
