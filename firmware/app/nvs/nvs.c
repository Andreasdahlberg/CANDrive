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

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/crc.h>
#include <assert.h>
#include <string.h>
#include "utility.h"
#include "logging.h"
#include "crc.h"
#include "nvs.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define NVS_LOGGER_NAME "NVS"
#ifndef NVS_LOGGER_DEBUG_LEVEL
#define NVS_LOGGER_DEBUG_LEVEL LOGGING_DEBUG
#endif

#define FLASH_START 0x08000000
#define FLASH_END 0x8020000
#define FLASH_MIN_NUMBER_OF_PAGES 2
#define FLASH_PAGE_SIZE 0x400

#define PAGE_HEADER_SIZE_WITHOUT_CRC (sizeof(struct nvs_page_header_t) - sizeof(uint32_t))

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct nvs_t
{
    uint32_t start_page_address;
    size_t number_of_pages;
    uint32_t active_page_address;
    uint32_t active_sequence_number;
    uint32_t active_address;
    logging_logger_t *logger_p;
};

enum nvs_page_state_t
{
    PAGE_ERASED = 0,
    PAGE_IN_USE = 0x0C00FFE0
};

enum nvs_item_status_t
{
    ITEM_DELETED = 0,
    ITEM_USED = 0xFFFF
};

struct nvs_page_header_t
{
    enum nvs_page_state_t state;
    uint32_t sequence_number;
    uint32_t crc;
};

struct nvs_item_t
{
    uint32_t hash;
    uint16_t size;
    uint16_t status;
    uint32_t crc;
    uint8_t data[0];
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct nvs_t self;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static bool ErasePage(uint32_t page_address);
static bool WriteToFlash(uint32_t address, const void *data_p, size_t length);
static void FindActivePage(void);
static void GetPageHeader(uint32_t address, struct nvs_page_header_t *page_header_p);
static uint32_t GetActiveAddress(void);
static uint32_t CalculateHash(const char *str_p);
static uint32_t CalculateItemCRC(const struct nvs_item_t *item_p);
static void ReadFromFlash(uint32_t address, void *data_p, size_t length);
static bool GetValueByHash(uint32_t page_address, uint32_t hash, uint32_t *value_p);
static void MoveItemsToNewPage(void);
static uint32_t GetNextPageAddress(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void NVS_Init(uint32_t start_page_address, size_t number_of_pages)
{
    assert(number_of_pages >= FLASH_MIN_NUMBER_OF_PAGES);

    self = (__typeof__(self)) {0};
    self.start_page_address = start_page_address;
    self.active_page_address = start_page_address;
    self.active_sequence_number = 0;
    self.number_of_pages = number_of_pages;
    self.logger_p = Logging_GetLogger(NVS_LOGGER_NAME);
    Logging_SetLevel(self.logger_p, NVS_LOGGER_DEBUG_LEVEL);

    FindActivePage();
    self.active_address = GetActiveAddress();

    struct nvs_page_header_t page_header;
    GetPageHeader(self.active_page_address, &page_header);
    const uint32_t crc = CRC_Calculate(&page_header, PAGE_HEADER_SIZE_WITHOUT_CRC);

    if ((page_header.state != PAGE_IN_USE) || (page_header.crc != crc))
    {
        Logging_Debug(self.logger_p, "Reset page: {page_address: 0x%x}", self.active_page_address);

        if(ErasePage(self.active_page_address))
        {
            page_header.state = PAGE_IN_USE;
            page_header.sequence_number = self.active_sequence_number;
            page_header.crc = CRC_Calculate(&page_header, PAGE_HEADER_SIZE_WITHOUT_CRC);
            WriteToFlash(self.active_page_address, &page_header, sizeof(page_header));
        }
    }

    Logging_Info(self.logger_p,
                 "NVS initialized: {page_address: 0x%x, sequence_number: %u, active_address: 0x%x}",
                 self.active_page_address,
                 self.active_sequence_number,
                 self.active_address);
}

bool NVS_Store(const char *key_p, uint32_t value)
{

    if ((sizeof(struct nvs_item_t) + sizeof(value)) > (FLASH_PAGE_SIZE - self.active_address))
    {
        MoveItemsToNewPage();
    }

    assert((sizeof(struct nvs_item_t) + sizeof(value)) <= (FLASH_PAGE_SIZE - self.active_address));

    struct nvs_item_t item;
    item.hash = CalculateHash(key_p);
    item.status = ITEM_USED;
    item.size = sizeof(value);
    item.crc = CalculateItemCRC(&item);

    const uint32_t destination = self.active_page_address + self.active_address;
    Logging_Debug(self.logger_p,
                  "Store: {key: %s, value: %u, hash: %u, size: %u, crc: 0x%x, destination: 0x%x}",
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
        self.active_address += sizeof(item) + sizeof(value);
    }
    else
    {
        Logging_Critical(self.logger_p, "Corrupt page: {page_address: 0x%x}", self.active_page_address);
    }

    return status;
}

bool NVS_Retrieve(const char *key_p, uint32_t *value_p)
{
    const uint32_t hash = CalculateHash(key_p);
    Logging_Debug(self.logger_p, "Retrieve: {key: %s, hash: %u}", key_p, hash);

    const bool status = GetValueByHash(self.active_page_address, hash, value_p);
    return status;
}

bool NVS_Remove(const char *key_p)
{
    bool status = false;
    const uint32_t hash = CalculateHash(key_p);

    uint32_t item_address = self.active_page_address + sizeof(struct nvs_page_header_t);
    while(item_address + sizeof(struct nvs_item_t) <= self.active_page_address + FLASH_PAGE_SIZE)
    {
        struct nvs_item_t item;
        ReadFromFlash(item_address, &item, sizeof(item));
        const uint32_t crc = CalculateItemCRC(&item);

        if ((item.hash == hash) && (item.crc == crc) && (item.status == ITEM_USED))
        {
            const uint16_t item_status = ITEM_DELETED;
            const size_t status_offset = 6;
            status = WriteToFlash(item_address + status_offset, &item_status, sizeof(item_status));

            if (!status)
            {
                status = false;
                Logging_Error(self.logger_p,
                              "Failed to remove item: {key: %s, hash: %u, size: %u}",
                              key_p,
                              item.hash,
                              item.size);
                break;
            }
        }
        else if (item.crc != crc)
        {
            /* Abort when all valid items are checked. */
            break;
        }
        else
        {
            /* Do nothing, item is valid but does not match the supplied key. */
        }
        item_address += sizeof(item) + item.size;
    }

    return status;
}

bool NVS_Clear(void)
{
    Logging_Info(self.logger_p,"Clear non volatile storage.");

    bool status = true;
    for (size_t i = 0; i < self.number_of_pages; ++i)
    {
        const uint32_t page_address = self.start_page_address + (i * FLASH_PAGE_SIZE);

        Logging_Debug(self.logger_p,"Erase page: {page_address: 0x%x}", page_address);
        if (!ErasePage(page_address))
        {
            Logging_Critical(self.logger_p, "Erase failed: {page_address: 0x%x}", page_address);
            status = false;
            break;
        }
    }

    NVS_Init(self.start_page_address, self.number_of_pages);
    return status;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static bool ErasePage(uint32_t page_address)
{
    bool status = true;
    flash_unlock();
    flash_erase_page(page_address);

    if(flash_get_status_flags() != FLASH_SR_EOP)
    {
        status = false;
        Logging_Error(self.logger_p,
                      "Failed erase page: {page_address: 0x%x, status_flags: 0x%x}",
                      page_address,
                      flash_get_status_flags());
    }

    flash_lock();
    flash_clear_status_flags();
    return status;
}

static void ReadFromFlash(uint32_t address, void *data_p, size_t length)
{
    const uint32_t *source_p = (const uint32_t *)((uintptr_t)address);
    memcpy(data_p, source_p, length);
}

static bool WriteToFlash(uint32_t address, const void *data_p, size_t length)
{
    bool status = true;

    assert(length % sizeof(uint16_t) == 0);

    flash_unlock();

    for (size_t i = 0; i < length / sizeof(uint16_t); ++i)
    {
        const uint32_t destination = address + (i * sizeof(uint16_t));
        const uint16_t data = *((const uint16_t *)data_p + i);

        flash_program_half_word(destination, data);
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

    flash_lock();
    flash_clear_status_flags();
    return status;
}

static void FindActivePage(void)
{
    for (size_t i = 0; i < self.number_of_pages; ++i)
    {
        const uint32_t address = self.start_page_address + (i * FLASH_PAGE_SIZE);
        struct nvs_page_header_t page_header;
        GetPageHeader(address, &page_header);

        const uint32_t crc = CRC_Calculate(&page_header, PAGE_HEADER_SIZE_WITHOUT_CRC);
        if ((page_header.state == PAGE_IN_USE) &&
                (page_header.sequence_number > self.active_sequence_number) &&
                (page_header.crc == crc))
        {
            self.active_sequence_number = page_header.sequence_number;
            self.active_page_address = address;
        }
    }
}

static void GetPageHeader(uint32_t address, struct nvs_page_header_t *page_header_p)
{
    ReadFromFlash(address, page_header_p, sizeof(*page_header_p));
}

static uint32_t GetActiveAddress(void)
{
    uint32_t active_address = self.active_page_address + sizeof(struct nvs_page_header_t);

    while(active_address + sizeof(struct nvs_item_t) <= self.active_page_address + FLASH_PAGE_SIZE)
    {
        struct nvs_item_t item;
        ReadFromFlash(active_address, &item, sizeof(item));

        if (item.crc != CalculateItemCRC(&item))
        {
            break;
        }

        active_address += sizeof(item) + item.size;
    }

    return active_address - self.active_page_address;
}

/**
 * Calculate FNV-1a hash for given string.
 */
static uint32_t CalculateHash(const char *str_p)
{
    const uint32_t fnv_offset = 2166136261;
    const uint32_t fnv_prime = 16777619;

    uint32_t hash = fnv_offset;
    for (const char *p = str_p; *p != '\0'; ++p)
    {
        hash ^= (uint32_t)(*p);
        hash *= fnv_prime;
    }
    return hash;
}

static uint32_t CalculateItemCRC(const struct nvs_item_t *item_p)
{
    const size_t item_size_without_crc = 6;
    return CRC_Calculate(item_p, item_size_without_crc);
}

static bool GetValueByHash(uint32_t page_address, uint32_t hash, uint32_t *value_p)
{
    bool status = false;
    uint32_t item_address = page_address + sizeof(struct nvs_page_header_t);
    while(item_address + sizeof(struct nvs_item_t) <= page_address + FLASH_PAGE_SIZE)
    {
        struct nvs_item_t item;
        ReadFromFlash(item_address, &item, sizeof(item));

        const uint32_t crc = CalculateItemCRC(&item);
        if ((item.hash == hash) && (item.crc == crc) && (item.status == ITEM_USED))
        {
            const uint32_t data_address = item_address + sizeof(item);
            ReadFromFlash(data_address, value_p, item.size);
            status = true;
        }
        else if (item.crc != crc)
        {
            /* Abort when all valid items are checked. */
            break;
        }
        else
        {
            /* Do nothing, item is valid but is removed or does not match the supplied hash. */
        }

        item_address += sizeof(item) + item.size;
    }

    return status;
}

static void MoveItemsToNewPage(void)
{
    Logging_Info(self.logger_p, "Move items to new page: {active_page_address: 0x%x, new_page_address: 0x%x}",
                 self.active_page_address,
                 GetNextPageAddress()
                );

    ErasePage(GetNextPageAddress());

    uint32_t destination = GetNextPageAddress() + sizeof(struct nvs_page_header_t);
    uint32_t item_address = self.active_page_address + sizeof(struct nvs_page_header_t);
    while(item_address + sizeof(struct nvs_item_t) <= self.active_page_address + FLASH_PAGE_SIZE)
    {
        struct nvs_item_t item;
        ReadFromFlash(item_address, &item, sizeof(item));

        const uint32_t crc = CalculateItemCRC(&item);
        if (item.crc == crc)
        {
            uint32_t value;
            GetValueByHash(self.active_page_address, item.hash, &value);

            if (!GetValueByHash(GetNextPageAddress(), item.hash, &value))
            {
                Logging_Debug(self.logger_p,
                              "Move: {value: %u, hash: %u, size: %u, crc: %u, destination: 0x%x}",
                              value,
                              item.hash,
                              item.size,
                              item.crc,
                              destination);
                WriteToFlash(destination, &item, sizeof(item));
                WriteToFlash(destination + sizeof(item), &value, sizeof(value));

                destination += sizeof(item) + sizeof(value);
            }
        }

        item_address += sizeof(item) + item.size;
    }

    struct nvs_page_header_t page_header;
    page_header.state = PAGE_IN_USE;
    page_header.sequence_number = self.active_sequence_number + 1;
    page_header.crc = CRC_Calculate(&page_header, PAGE_HEADER_SIZE_WITHOUT_CRC);
    WriteToFlash(GetNextPageAddress(), &page_header, sizeof(page_header));

    self.active_sequence_number = page_header.sequence_number;
    self.active_page_address = GetNextPageAddress();
    self.active_address = destination - self.active_page_address;

    Logging_Info(self.logger_p,
                 "Items moved to new page: {page_address: 0x%x, sequence_number: %u, active_address: 0x%x}",
                 self.active_page_address,
                 self.active_sequence_number,
                 self.active_address);
}

static uint32_t GetNextPageAddress(void)
{
    uint32_t next_page_address = self.active_page_address + FLASH_PAGE_SIZE;

    if (next_page_address >= (self.start_page_address + (self.number_of_pages * FLASH_PAGE_SIZE)))
    {
        next_page_address = self.start_page_address;
    }
    return  next_page_address;
}
