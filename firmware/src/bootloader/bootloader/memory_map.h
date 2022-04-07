/**
 * @file   memory_map.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Memory map.
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

#ifndef MEMORY_MAP_H_
#define MEMORY_MAP_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>

//////////////////////////////////////////////////////////////////////////
//EXTERNAL VARIABLES
//////////////////////////////////////////////////////////////////////////

extern uintptr_t __bootrom_start__;
extern uintptr_t __bootrom_size__;
extern uintptr_t __approm_start__;
extern uintptr_t __approm_size__;
extern uintptr_t __nvsrom_start__;
extern uintptr_t __nvsrom_size__;

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

#endif
