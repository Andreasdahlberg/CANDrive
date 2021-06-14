/**
 * @file   logging.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Logging module.
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

#ifndef UTILITY_H_
#define UTILITY_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stddef.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define ElementsIn(array) \
    ({ \
        _Static_assert \
        ( \
            ! __builtin_types_compatible_p(__typeof__(array), __typeof__(&array[0])), \
            "ElementsIn: "  # array " is not an array" \
        ); \
        sizeof(array) / sizeof((array)[0]); \
    })

#define STRINGIFY(x) #x

#if defined(__clang__)
#define NO_OPTIMIZATION_ATTRIBUTE optnone
#else
#define NO_OPTIMIZATION_ATTRIBUTE optimize("O0")
#endif

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////
/**
 * Copy strings in a safe way.
 *
 * @param  dst_p Pointer to destination buffer,
 * @param  src_p Pointer to string.
 * @param  bufsize Size of destination buffer.
 *
 * @return Total length of the string.
 */
size_t CopyString(char *dst_p, const char *src_p, size_t bufsize);

#endif
