/**
 * @file   crc.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Implementation of CRC module.
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

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/crc.h>
#include <assert.h>
#include <string.h>
#include "crc.h"

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
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

uint32_t CRC_Calculate(const void *data_p, size_t length)
{
    assert(data_p != NULL);

    uint32_t result = 0;
    const uint32_t word_size = sizeof(uint32_t);
    const uint32_t number_of_words = length / word_size;

    rcc_periph_clock_enable(RCC_CRC);
    crc_reset();

    const uint8_t *temp_p = (uint8_t *)data_p;
    for (size_t i = 0; i < number_of_words; ++i)
    {
        uint32_t data;
        memcpy(&data, temp_p, word_size);
        result = crc_calculate(data);

        temp_p += word_size;
    }

    uint32_t remainder = length % word_size;
    if (remainder != 0)
    {
        uint32_t data;
        memcpy(&data, temp_p, remainder);
        result = crc_calculate(data);
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
