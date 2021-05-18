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

#include <libopencm3/stm32/usart.h>
#include <errno.h>

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

int _write(int file, char *ptr, int len);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int _write(int file, char *ptr, int len)
{
    int i;

    if (file == 1)
    {
        for (i = 0; i < len; i++)
        {
            usart_send_blocking(USART2, ptr[i]);
        }
        return i;
    }

    errno = EIO;

    return -1;
}
