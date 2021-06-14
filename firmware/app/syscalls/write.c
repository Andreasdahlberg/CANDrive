/**
 * @file   write.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Implementation of the write syscall.
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

#include <unistd.h>
#include <errno.h>
#include "serial.h"

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

ssize_t _write(uint32_t file, char *ptr, size_t len);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

ssize_t _write(uint32_t file, char *ptr, size_t len)
{
    if ((file == STDOUT_FILENO) || (file == STDERR_FILENO))
    {
        Serial_Send(ptr, len);
        return len;
    }

    errno = EIO;

    return -1;
}
