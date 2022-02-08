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

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "utility.h"
#include "logging.h"
#include "systime.h"
#include "config.h"
#include "crc.h"
#include "isotp.h"
#include "protocol.h"
#include "board.h"
#include "flash.h"
#include "firmware_manager.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define FIRMWAREMANAGER_LOGGER_NAME "FwMan"
#ifndef FIRMWAREMANAGER_LOGGER_DEBUG_LEVEL
#define FIRMWAREMANAGER_LOGGER_DEBUG_LEVEL LOGGING_INFO
#endif

#define RX_BUFFER_SIZE 1152
#define TX_BUFFER_SIZE 64
#define PAGE_SIZE 1024
_Static_assert(RX_BUFFER_SIZE > PAGE_SIZE, "The RX-buffer must have space for an entire page!");

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

enum download_state_t
{
    IDLE = 0,
    ACTIVE
};

struct payload_info_t
{
    uint32_t size;
    uint32_t received_bytes;
    uint32_t crc;
    enum download_state_t state;
};

struct module_t
{
    logging_logger_t *logger_p;
    struct payload_info_t payload;
    struct isotp_ctx_t ctx;
    uint8_t rx_buffer[RX_BUFFER_SIZE];
    uint8_t tx_buffer[TX_BUFFER_SIZE];
    uint32_t page_index;
    bool tx_active;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

extern struct firmware_information_t firmware_information;
static struct module_t module;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static void RxStatusCallback(enum isotp_status_t status);
static void TxStatusCallback(enum isotp_status_t status);
static void HandleMessage(void);
static void OnReqFirmwareInformation(void);
static void OnReqReset(void);
static void OnFirmwareHeader(const struct message_header_t *message_header_p);
static void OnFirmwareData(const struct message_header_t *message_header_p);
static uint32_t GetPageAddress(uint32_t page_index);
static void StoreData(uint32_t address, uint8_t *data_p, size_t length);
static void UpdatePageIndex(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void FirmwareManager_Init(void)
{
    module = (__typeof__(module)) {0};

    module.logger_p = Logging_GetLogger(FIRMWAREMANAGER_LOGGER_NAME);
    Logging_SetLevel(module.logger_p, FIRMWAREMANAGER_LOGGER_DEBUG_LEVEL);

    ISOTP_Bind(&module.ctx,
               module.rx_buffer,
               sizeof(module.rx_buffer),
               (uint16_t)Config_GetValue("rx_id"),
               (uint16_t)Config_GetValue("tx_id"),
               0,
               RxStatusCallback,
               TxStatusCallback
              );

    Logging_Info(module.logger_p, "Firmware manager initialized: {version: %u, addr: %x, up_addr: %x}",
                 firmware_information.version,
                 Board_GetApplicationAddress(),
                 Board_GetUpgradeMemoryAddress());
}

void FirmwareManager_Update(void)
{
    ISOTP_Proccess(&module.ctx);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static void RxStatusCallback(enum isotp_status_t status)
{
    switch(status)
    {
        case ISOTP_STATUS_DONE:
            HandleMessage();
            break;

        case ISOTP_STATUS_WAITING:
            /**
             * Do nothing here, waiting for space in RX-buffer. No need to check for
             * timeout since it's handled by the ISO-TP layer.
             */
            break;

        case ISOTP_STATUS_TIMEOUT:
        case ISOTP_STATUS_LOST_FRAME:
        case ISOTP_STATUS_OVERFLOW_ABORT:
            Logging_Warning(module.logger_p, "Failed to receive: {status: %u}", (uint32_t)status);
            module.payload.state = IDLE;
            break;
        default:
            Logging_Warning(module.logger_p, "Unknown status: {status: %u}", (uint32_t)status);
            module.payload.state = IDLE;
            break;
    }
}

static void TxStatusCallback(enum isotp_status_t status)
{
    switch(status)
    {
        case ISOTP_STATUS_DONE:
            Logging_Debug(module.logger_p, "Send done!");
            module.tx_active = false;
            break;
        case ISOTP_STATUS_TIMEOUT:
        case ISOTP_STATUS_LOST_FRAME:
        case ISOTP_STATUS_OVERFLOW_ABORT:
            Logging_Warning(module.logger_p, "Failed to send: {status: %u}", (uint32_t)status);
            module.tx_active = false;
            break;
        default:
            Logging_Warning(module.logger_p, "Unknown status: {status: %u}", (uint32_t)status);
            module.tx_active = false;
            break;
    }
}

static void HandleMessage(void)
{
    struct message_header_t header;
    const size_t number_of_bytes = ISOTP_Receive(&module.ctx, &header, sizeof(header));
    if (number_of_bytes == sizeof(header))
    {
        const uint32_t header_crc = CRC_Calculate(&header, sizeof(header) - sizeof(header.header_crc));
        Logging_Debug(module.logger_p,
                      "header: {type: %u, size: %u, crc: %x, expected_crc: %x}",
                      header.type,
                      header.size,
                      header.header_crc,
                      header_crc);

        if (header.header_crc == header_crc)
        {
            switch (header.type)
            {
                case REQ_FW_INFO:
                    OnReqFirmwareInformation();
                    break;
                case REQ_RESET:
                    OnReqReset();
                    break;
                case REQ_FW_HEADER:
                    OnFirmwareHeader(&header);
                    break;
                case REQ_FW_DATA:
                    OnFirmwareData(&header);
                    break;
                default:
                    Logging_Warning(module.logger_p, "Unknown type: {type: %u}", header.type);
                    break;
            }
        }
        else
        {
            Logging_Error(module.logger_p, "CRC mismatch: {crc: %x, expected_crc: %x}", header.header_crc, header_crc);
        }
    }
    else
    {
        Logging_Error(module.logger_p, "Incomplete header: {size: %u}", number_of_bytes);
    }
}

static void OnReqFirmwareInformation(void)
{
    struct firmware_info_msg_t info =
    {
        .type = REQ_FW_INFO,
        .version = firmware_information.version,
        .hardware_revision = Board_GetHardwareRevision(),
        .name = NAME,
        .offset = Board_GetUpgradeMemoryAddress(),
        .id = {0, 0, 0}
    };

    struct board_id_t id = Board_GetId();
    memcpy(info.id, &id, sizeof(info.id));

    memcpy(module.tx_buffer, &info, sizeof(info));
    ISOTP_Send(&module.ctx, module.tx_buffer, sizeof(info));
    module.tx_active = true;
}

static void OnReqReset(void)
{
    Logging_Info(module.logger_p, "Restart on request");
    Board_Reset();
}

static void OnFirmwareHeader(const struct message_header_t *message_header_p)
{
    struct firmware_image_t image;
    const size_t number_of_bytes = ISOTP_Receive(&module.ctx, &image, sizeof(image));
    if (number_of_bytes == sizeof(image))
    {
        const uint32_t crc = CRC_Calculate(&image, sizeof(image));
        Logging_Info(module.logger_p,
                     "Download started: {size: %u, data_crc: 0x%x, crc: 0x%x, expected_crc: 0x%x}",
                     image.size,
                     image.crc,
                     message_header_p->payload_crc,
                     crc);

        if (message_header_p->payload_crc == crc)
        {
            const uint32_t page_address = GetPageAddress(0);
            if (Flash_ErasePage(page_address))
            {
                module.payload.size = image.size;
                module.payload.crc = image.crc;
                module.payload.received_bytes = 0;
                module.payload.state = ACTIVE;
                module.page_index = 0;
            }
        }
        else
        {
            Logging_Error(module.logger_p, "CRC mismatch: {crc: %x, expected_crc: %x}", message_header_p->payload_crc, crc);
        }
    }
}

static uint32_t GetPageAddress(uint32_t page_index)
{
    return page_index * PAGE_SIZE + Board_GetUpgradeMemoryAddress();
}

static void OnFirmwareData(const struct message_header_t *message_header_p __attribute__((unused)))
{
    uint8_t data[128];
    _Static_assert(PAGE_SIZE % (sizeof(data)) == 0, "Invalid buffer size");

    size_t number_of_bytes;
    while (module.payload.state == ACTIVE)
    {
        number_of_bytes = ISOTP_Receive(&module.ctx, data, sizeof(data));
        if (number_of_bytes == 0)
        {
            break;
        }

        const uint32_t address = Board_GetUpgradeMemoryAddress() + module.payload.received_bytes;
        const uint32_t number_of_pages = (module.payload.size + PAGE_SIZE - 1) / PAGE_SIZE;
        const uint32_t page_index = (module.payload.received_bytes) / PAGE_SIZE;
        Logging_Debug(module.logger_p, "data: {received_bytes: %u, pages: %u, page_index: %u, address: %x}", module.payload.received_bytes, number_of_pages, page_index, address);

        module.payload.received_bytes += number_of_bytes;
        StoreData(address, data, number_of_bytes);
    }
}

static void StoreData(uint32_t address, uint8_t *data_p, size_t length)
{
    if (Flash_Write(address, data_p, length))
    {
        if (module.payload.received_bytes >= module.payload.size)
        {
            Logging_Info(module.logger_p, "Download complete");
            module.payload.state = IDLE;
        }
        else
        {
            UpdatePageIndex();
        }
    }
    else
    {
        Logging_Error(module.logger_p, "Abort download");
        module.payload.state = IDLE;
    }
}

static void UpdatePageIndex(void)
{
    const uint32_t next_page_index = (module.payload.received_bytes) / PAGE_SIZE;
    if (module.page_index != next_page_index)
    {
        const uint32_t page_address = GetPageAddress(next_page_index);
        if (Flash_ErasePage(page_address))
        {
            module.page_index = next_page_index;
        }
        else
        {
            Logging_Error(module.logger_p, "Abort download");
            module.payload.state = IDLE;
        }
    }
}
