/**
 * @file   mock_flash.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for flash.
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

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <string.h>
#include <libopencm3/stm32/flash.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define NUMBER_OF_PAGES 2
#define PAGE_SIZE 1024

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static uint32_t flash_data[NUMBER_OF_PAGES][PAGE_SIZE];

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void *__real_memcpy (void *destination_p, const void *source_p, size_t length);

void *__wrap_memcpy (void *destination_p, const void *source_p, size_t length)
{
    uint8_t *real_source_p = ((uint8_t *)flash_data) + ((uintptr_t )source_p);
    return __real_memcpy(destination_p, real_source_p, length);
}

__attribute__((weak)) void flash_halfcycle_enable(void)
{

}

__attribute__((weak)) void flash_halfcycle_disable(void)
{

}

__attribute__((weak)) void flash_prefetch_enable(void)
{

}

__attribute__((weak)) void flash_prefetch_disable(void)
{

}

__attribute__((weak)) void flash_set_ws(uint32_t ws)
{

}

__attribute__((weak)) void flash_lock(void)
{

}

__attribute__((weak)) void flash_unlock(void)
{

}

__attribute__((weak)) void flash_unlock_option_bytes(void)
{

}

__attribute__((weak)) void flash_clear_pgerr_flag(void)
{

}

__attribute__((weak)) void flash_clear_wrprterr_flag(void)
{

}

__attribute__((weak)) uint32_t flash_get_status_flags(void)
{
    return mock_type(uint32_t);
}

__attribute__((weak)) void flash_clear_status_flags(void)
{

}

__attribute__((weak)) void flash_program_word(uint32_t address, uint32_t data)
{
    uint8_t *destination_p = ((uint8_t *)flash_data) + address;
    __real_memcpy(destination_p, &data, sizeof(data));
}

__attribute__((weak)) void flash_program_half_word(uint32_t address, uint16_t data)
{

}

__attribute__((weak)) void flash_erase_page(uint32_t page_address)
{
    uint32_t page_index = page_address / PAGE_SIZE;
    memset(flash_data[page_index], 0xFF, PAGE_SIZE);
}

__attribute__((weak)) void flash_erase_all_pages(void)
{

    memset(flash_data, 0xFF, sizeof(flash_data));
}

__attribute__((weak)) void flash_erase_option_bytes(void)
{

}

__attribute__((weak)) void flash_program_option_bytes(uint32_t address, uint16_t data)
{

}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
