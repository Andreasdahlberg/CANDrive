/**
 * @file   nvs.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Non volatile storage module.
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
#include "logging.h"
#include "nvs.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define NVS_LOGGER_NAME "NVS"
#ifndef NVS_LOGGER_DEBUG_LEVEL
#define NVS_LOGGER_DEBUG_LEVEL LOGGING_DEBUG
#endif

//#define FLASH_START_ADDRESS ((uint32_t)0x08000000)
//#define FLASH_OPERATION_ADDRESS ((uint32_t)0x0801FC00)

#define FLASH_PAGE_NUM_MAX 127
#define FLASH_PAGE_SIZE 0x400

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct nvs_t
{
    uint32_t start_page_address;
    size_t number_of_pages;
    uint32_t active_page_address;
    uint32_t active_sequence_number;
    uint32_t address;
    logging_logger_t *logger_p;
};

enum nvs_page_state_t
{
    PAGE_ERASED = 0,
    PAGE_IN_USE = 0x0C00FFE0
};

struct nvs_page_header_t
{
    /* TODO: Enforce 32-bit? */
    enum nvs_page_state_t state;
    uint32_t sequence_number;
    uint32_t crc;
};

struct nvs_item_t
{
    uint32_t hash;
    uint32_t size;
    uint32_t crc;
    uint8_t data[0];
};

/* TODO: Add static assert, sizeof(header) % sizeof(uint32_t) == 0 */

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct nvs_t self;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////
static bool ErasePage(uint32_t page_address);
static void ReadFromFlash(size_t address, void *data_p, size_t length);
static bool WriteToFlash(uint32_t address, const void *data_p, size_t length);
static void GetPageHeader(uint32_t address, struct nvs_page_header_t *page_header_p);
static uint32_t CalculateHash(const char *str_p);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void NVS_Init(uint32_t start_page_address, size_t number_of_pages)
{
    /*TODO: Add assertion to check if addresses are valid. */

    self = (__typeof__(self)) {0};
    self.start_page_address = start_page_address;
    self.number_of_pages = number_of_pages;
    self.logger_p = Logging_GetLogger(NVS_LOGGER_NAME);
    Logging_SetLevel(self.logger_p, NVS_LOGGER_DEBUG_LEVEL);

    for (size_t i = 0; i < self.number_of_pages; ++i)
    {
        const uint32_t address = self.start_page_address + (i * FLASH_PAGE_SIZE);
        struct nvs_page_header_t page_header;
        GetPageHeader(address, &page_header);

        /* TODO: Check CRC, maybe create function IsPageInUse() */
        if ((page_header.state == PAGE_IN_USE) && (page_header.sequence_number > self.active_sequence_number))
        {
            self.active_sequence_number = page_header.sequence_number;
            self.active_page_address = address;
        }
    }

    struct nvs_page_header_t page_header;
    GetPageHeader(self.active_page_address, &page_header);
    /* TODO: Check CRC */
    if (page_header.state != PAGE_IN_USE)
    {
        Logging_Debug(self.logger_p, "Reset page: {page_address: 0x%x}", self.active_page_address);

        if(ErasePage(self.active_page_address))
        {
            page_header.state = PAGE_IN_USE;
            page_header.sequence_number = self.active_sequence_number;
            page_header.crc = 0; /* TODO: Calculate CRC */
            WriteToFlash(self.active_page_address, &page_header, sizeof(page_header));

        }
    }

    /* TODO: Handle flash failures */
    Logging_Info(self.logger_p,
                 "NVS initialized: {page_address: 0x%x, sequence_number: %u}",
                 self.active_page_address,
                 self.active_sequence_number);

}

bool NVS_Store(const char *key_p, uint32_t value)
{
    /* TODO: move to new page if current page is full */
    assert((sizeof(struct nvs_item_t) + sizeof(value)) <= (FLASH_PAGE_SIZE - self.address));

    struct nvs_item_t item;
    item.hash = CalculateHash(key_p);
    item.size = sizeof(value);
    item.crc = 0;

    const uint32_t destination = self.active_page_address + self.address;
    Logging_Debug(self.logger_p,
                  "Store: {key: %s, value: %u, hash: %u, size: %u, crc: %u, destination: %u}",
                  key_p,
                  value,
                  item.hash,
                  item.size,
                  item.crc,
                  destination);

    const bool status = WriteToFlash(destination, &item, sizeof(item)) &&
                        WriteToFlash(destination + sizeof(item), &value, sizeof(value));

    if (status)
    {
        self.address = destination + sizeof(value);
    }
    else
    {
        /* TODO: Handle corrupt page, move to new page and erase. */
        Logging_Debug(self.logger_p, "Corrupt page: {page_address: 0x%x}", self.active_page_address);
    }

    return status;
}

bool NVS_Retrieve(const char *key_p, uint32_t *value_p)
{
    bool status = false;
    const uint32_t hash = CalculateHash(key_p);

    uint32_t item_address = self.active_page_address + sizeof(struct nvs_page_header_t);

    while(item_address < self.active_page_address + FLASH_PAGE_SIZE)
    {
        struct nvs_item_t item;
        ReadFromFlash(item_address, &item, sizeof(item));

        /* TODO: Check crc */
        if ((item.hash == hash) && (item.crc == 0))
        {
            const uint32_t data_address = item_address + sizeof(item);
            ReadFromFlash(data_address, value_p, item.size);
            status = true;
        }

        item_address += sizeof(item) + item.size;
    }

    return status;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static bool ErasePage(uint32_t page_address)
{
    bool status = true;
    flash_erase_page(page_address);

    if(flash_get_status_flags() == FLASH_SR_EOP)
    {
        status = false;
        Logging_Error(self.logger_p,
                      "Failed erase page: {page_address: 0x%x, status_flags: 0x%x}",
                      page_address,
                      flash_get_status_flags());
    }

    return status;
}

static void ReadFromFlash(size_t address, void *data_p, size_t length)
{
    const volatile uint8_t *source_p= (const volatile uint8_t *)address;
    uint8_t *destination_p = (uint8_t *)data_p;

    for (size_t i = 0; i < length; ++i)
    {
        *destination_p = *source_p;

        destination_p += 1;
        source_p += 1;
    }
}

static bool WriteToFlash(uint32_t address, const void *data_p, size_t length)
{
    bool status = true;

    assert(length % sizeof(uint32_t) == 0);

    for (size_t i = 0; i < length / sizeof(uint32_t); ++i)
    {
        const uint32_t destination = address + (i * sizeof(uint32_t));
        const uint32_t data = *((const uint32_t *)data_p + i);

        flash_program_word(destination, data);
        if(flash_get_status_flags() != FLASH_SR_EOP)
        {
            status = false;
            Logging_Error(self.logger_p,
                          "Failed write to flash: {address: 0x%x, status_flags: 0x%x}",
                          destination,
                          flash_get_status_flags());
            break;
        }
    }

    return status;
}

static void GetPageHeader(uint32_t address, struct nvs_page_header_t *page_header_p)
{
    ReadFromFlash(address, page_header_p, sizeof(*page_header_p));
}


static uint32_t CalculateHash(const char *str_p)
{
    uint32_t CalculateHash = 5381;
    uint32_t c;

    while (c = *str_p++)
    {
        CalculateHash = ((CalculateHash << 5) + CalculateHash) + c;
    }

    return CalculateHash;
}
