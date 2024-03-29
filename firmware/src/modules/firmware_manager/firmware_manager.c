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
#include "board.h"
#include "utility.h"
#include "logging.h"
#include "systime.h"
#include "config.h"
#include "crc.h"
#include "isotp.h"
#include "protocol.h"
#include "board.h"
#include "flash.h"
#include "image.h"
#include "nvcom.h"
#include "firmware_manager.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define FIRMWAREMANAGER_LOGGER_NAME "FwMan"
#ifndef FIRMWAREMANAGER_LOGGER_DEBUG_LEVEL
#define FIRMWAREMANAGER_LOGGER_DEBUG_LEVEL LOGGING_INFO
#endif

#define RX_ID 0x1
#define TX_ID 0x2
#define RX_BUFFER_SIZE 1152
#define TX_BUFFER_SIZE 128
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
    firmware_manager_allowed_t reset_allowed_func;
    firmware_manager_allowed_t update_allowed_func;
    firmware_manager_reset_t reset_func;
    struct payload_info_t payload;
    struct isotp_ctx_t ctx;
    uint8_t rx_buffer[RX_BUFFER_SIZE];
    uint8_t tx_buffer[TX_BUFFER_SIZE];
    uint32_t page_index;
    bool active;
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
static void OnReqUpdate(void);
static void OnFirmwareHeader(const struct message_header_t *message_header_p);
static void OnFirmwareData(const struct message_header_t *message_header_p);
static uint32_t GetPageAddress(uint32_t page_index);
static void StoreData(uint32_t address, const uint8_t *data_p, size_t length);
static void UpdatePageIndex(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void FirmwareManager_Init(firmware_manager_reset_t reset)
{
    module = (__typeof__(module)) {0};

    module.active = true;
    module.reset_func = reset;
    module.logger_p = Logging_GetLogger(FIRMWAREMANAGER_LOGGER_NAME);
    Logging_SetLevel(module.logger_p, FIRMWAREMANAGER_LOGGER_DEBUG_LEVEL);

    ISOTP_Bind(&module.ctx,
               module.rx_buffer,
               sizeof(module.rx_buffer),
               module.tx_buffer,
               sizeof(module.tx_buffer),
               RX_ID,
               TX_ID,
               RxStatusCallback,
               TxStatusCallback
              );


    Logging_Info(module.logger_p, "Firmware manager initialized");
}

void FirmwareManager_SetActionChecks(firmware_manager_allowed_t reset, firmware_manager_allowed_t update)
{
    module.reset_allowed_func = reset;
    module.update_allowed_func = update;
}

void FirmwareManager_Update(void)
{
    ISOTP_Proccess(&module.ctx);
}

bool FirmwareManager_Active(void)
{
    return module.active;
}

bool FirmwareManager_DownloadActive(void)
{
    return module.payload.state == ACTIVE;
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
            break;
        case ISOTP_STATUS_WAITING:
            /**
             * Do nothing here, wait for receiver. No need to check for timeout
             * since it's handled by the ISO-TP layer.
             */
            break;
        case ISOTP_STATUS_TIMEOUT:
        case ISOTP_STATUS_LOST_FRAME:
        case ISOTP_STATUS_OVERFLOW_ABORT:
            Logging_Warning(module.logger_p, "Failed to send: {status: %u}", (uint32_t)status);
            break;
        default:
            Logging_Warning(module.logger_p, "Unknown status: {status: %u}", (uint32_t)status);
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
                case REQ_UPDATE:
                    OnReqUpdate();
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
    Logging_Debug(module.logger_p, "ReqFirmwareInformation");

    struct firmware_info_msg_t info =
    {
        .type = REQ_FW_INFO,
        .version = "None",
        .hardware_revision = Board_GetHardwareRevision(),
        .name = "None",
        .id = {0, 0, 0},
        .git_sha = "None"

    };

    struct board_id_t id = Board_GetId();
    memcpy(info.id, &id, sizeof(info.id));

    const struct image_header_t *header_p = Image_GetHeader((uintptr_t *)Board_GetApplicationAddress());
    if ((header_p != NULL) && (Image_IsValid((uintptr_t *)Board_GetApplicationAddress())))
    {
        CopyString(info.version, header_p->version, sizeof(info.version));
        CopyString(info.name, Image_TypeToString(header_p->image_type), sizeof(info.name));
        CopyString(info.git_sha, header_p->git_sha, sizeof(info.git_sha));
    }

    if (!ISOTP_Send(&module.ctx, &info, sizeof(info)))
    {
        Logging_Error(module.logger_p, "Failed to send: {type: %u}", info.type);
    }
}

static void OnReqReset(void)
{
    if ((module.reset_allowed_func == NULL) || module.reset_allowed_func())
    {
        Logging_Info(module.logger_p, "Restarting...");
        module.active = false;
        if (module.reset_func != NULL)
        {
            module.reset_func();
        }
    }
    else
    {
        Logging_Warning(module.logger_p, "Reset not allowed");
    }
}

static void OnReqUpdate(void)
{
    if ((module.update_allowed_func == NULL) || module.update_allowed_func())
    {
        Logging_Info(module.logger_p, "Request update...");
        struct nvcom_data_t *data_p = NVCom_GetData();
        data_p->request_firmware_update = true;
        NVCom_SetData(data_p);
    }
    else
    {
        Logging_Warning(module.logger_p, "Update not allowed");
    }
}

static void OnFirmwareHeader(const struct message_header_t *message_header_p)
{
    if ((module.update_allowed_func == NULL) || module.update_allowed_func())
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
    else
    {
        Logging_Warning(module.logger_p, "Update not allowed");
    }
}

static uint32_t GetPageAddress(uint32_t page_index)
{
    return page_index * PAGE_SIZE + Board_GetApplicationAddress();
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

        const uint32_t address = (uint32_t)Board_GetApplicationAddress() + module.payload.received_bytes;
        const uint32_t number_of_pages = (module.payload.size + PAGE_SIZE - 1) / PAGE_SIZE;
        const uint32_t page_index = (module.payload.received_bytes) / PAGE_SIZE;
        Logging_Debug(module.logger_p, "data: {received_bytes: %u, pages: %u, page_index: %u, address: %x}", module.payload.received_bytes, number_of_pages, page_index, address);

        module.payload.received_bytes += number_of_bytes;
        StoreData(address, data, number_of_bytes);
    }
}

static void StoreData(uint32_t address, const uint8_t *data_p, size_t length)
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
