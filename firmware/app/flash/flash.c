/**
 * @file   firmware_manager.c
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

#include <libopencm3/stm32/flash.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "utility.h"
#include "logging.h"
#include "flash.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define FLASH_LOGGER_NAME "Flash"
#ifndef FLASH_LOGGER_DEBUG_LEVEL
#define FLASH_LOGGER_DEBUG_LEVEL LOGGING_DEBUG
#endif

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct module_t
{
    logging_logger_t *logger_p;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct module_t module;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static bool ProgramWord(uint32_t address, uint32_t data);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void Flash_Init(void)
{
    module = (__typeof__(module)) {0};
    module.logger_p = Logging_GetLogger(FLASH_LOGGER_NAME);
    Logging_SetLevel(module.logger_p, FLASH_LOGGER_DEBUG_LEVEL);

    flash_clear_status_flags();
}

bool Flash_Write(uint32_t address, const void *data_p, size_t length)
{
    bool status = true;
    const uint32_t word_size = sizeof(uint32_t);
    const uint32_t number_of_words = length / word_size;

    flash_unlock();

    const uint8_t *temp_p = (const uint8_t *)data_p;
    for (size_t i = 0; i < number_of_words; ++i)
    {
        const uint32_t destination = address + (i * word_size);
        uint32_t data;
        memcpy(&data, temp_p, word_size);
        temp_p += word_size;

        status = ProgramWord(destination, data);
        if(!status)
        {
            break;
        }
    }

    const uint32_t number_of_remaining_bytes = length % word_size;
    if (status && (number_of_remaining_bytes != 0))
    {
        const uint32_t destination = address + (number_of_words * word_size);
        uint32_t data = 0;
        memcpy(&data, temp_p, number_of_remaining_bytes);

        status = ProgramWord(destination, data);
    }

    flash_lock();
    return status;
}

bool Flash_ErasePage(uint32_t page_address)
{
    Logging_Debug(module.logger_p, "Erase page 0x%x", page_address);

    flash_unlock();
    flash_erase_page(page_address);

    bool status = true;
    if(flash_get_status_flags() != FLASH_SR_EOP)
    {
        status = false;
        Logging_Error(module.logger_p,
                      "Failed erase page: {page_address: 0x%x, status_flags: 0x%x}",
                      page_address,
                      flash_get_status_flags());
    }

    flash_clear_status_flags();
    flash_lock();
    return status;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static bool ProgramWord(uint32_t address, uint32_t data)
{
    bool status = true;
    flash_program_word(address, data);
    if(flash_get_status_flags() != FLASH_SR_EOP)
    {
        status = false;
        Logging_Error(module.logger_p,
                      "Failed to write {address: 0x%x, status_flags: 0x%x}",
                      address,
                      flash_get_status_flags());
    }

    flash_clear_status_flags();
    return status;
}
