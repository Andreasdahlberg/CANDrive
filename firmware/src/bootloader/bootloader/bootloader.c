/**
 * @file   bootloader.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  CANDrive bootloader.
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

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/vector.h>
#include "memory_map.h"
#include "board.h"
#include "serial.h"
#include "systime.h"
#include "logging.h"
#include "image.h"
#include "can_interface.h"
#include "flash.h"
#include "firmware_manager.h"
#include "nvcom.h"
#include "bootloader.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define BOOTLOADER_LOGGER_NAME "Boot"
#ifndef BOOTLOADER_LOGGER_DEBUG_LEVEL
#define BOOTLOADER_LOGGER_DEBUG_LEVEL LOGGING_INFO
#endif

#ifndef GIT_DESC
#define GIT_DESC "UNKNOWN"
#endif

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct module_t
{
    logging_logger_t *logger;
    uint32_t status_led_last_update;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct module_t module;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static void UpdateRestartInformation(void);
static inline void StartApplication(const uintptr_t *start_p);
static inline void PrepareForApplication(const struct image_header_t *header_p);
static void JumpToApplication(void *pc, void *sp) __attribute__((naked, noreturn));
static inline void UpdateFirmware(void);
static bool IsUpdateRequested(void);
static void ClearUpdateRequest(void);
static inline bool IsWatchdogRestart(void);
static void UpdateStatusLED(void);
static void Reset(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void Bootloader_Init(void)
{
    Board_Init();
    SysTime_Init();
    Serial_Init(BAUD_RATE);
    Logging_Init(SysTime_GetSystemTime);
    NVCom_Init();
    Image_Init();

    module.logger = Logging_GetLogger(BOOTLOADER_LOGGER_NAME);
    Logging_SetLevel(module.logger, BOOTLOADER_LOGGER_DEBUG_LEVEL);
    Logging_Info(module.logger, "Bootloader ready: {sha: %s}", GIT_DESC);
}

void Bootloader_Start(void)
{
    UpdateRestartInformation();

    if (IsUpdateRequested())
    {
        Logging_Info(module.logger, "Firmware update requested");
        UpdateFirmware();
    }
    else
    {
        StartApplication(&__approm_start__);
        Logging_Error(module.logger, "Failed to start application");
        UpdateFirmware();
    }
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static void UpdateRestartInformation(void)
{
    struct nvcom_data_t *data_p = NVCom_GetData();

    if (data_p->number_of_restarts < UINT16_MAX)
    {
        data_p->number_of_restarts += 1;
    }
    data_p->reset_flags = Board_GetResetFlags();

    if (IsWatchdogRestart())
    {
        if (data_p->number_of_watchdog_restarts < UINT16_MAX)
        {
            data_p->number_of_watchdog_restarts += 1;
        }
    }
    else
    {
        data_p->number_of_watchdog_restarts = 0;
    }

    NVCom_SetData(data_p);
}

static inline void StartApplication(const uintptr_t *start_p)
{
    const struct image_header_t *header_p = Image_GetHeader(start_p);
    if (header_p != NULL && Image_IsValid(start_p))
    {
        PrepareForApplication(header_p);

        Logging_Debug(module.logger, "image: {type: %s, version: %s, sha: %s, crc: %u, size: %u}",
                      Image_TypeToString(header_p->image_type),
                      header_p->version,
                      header_p->git_sha,
                      header_p->crc,
                      header_p->size
                     );

        const vector_table_t *vector_table_p = (const vector_table_t *)header_p->vector_address;
        JumpToApplication(vector_table_p->reset, vector_table_p->initial_sp_value);
    }
    else
    {
        Logging_Error(module.logger, "Invalid firmware");
    }
}

static inline void PrepareForApplication(const struct image_header_t *header_p)
{
    const vector_table_t *vector_table_p = (const vector_table_t *)header_p->vector_address;
    SCB_VTOR = (uintptr_t)vector_table_p;
}

static void JumpToApplication(__attribute__((unused)) void *pc, __attribute__((unused)) void *sp)
{
    __asm("MSR MSP,r1");
    __asm("BX r0");
}

static inline void UpdateFirmware()
{
    CANInterface_Init();
    Flash_Init();
    FirmwareManager_Init(Reset);

    Logging_Info(module.logger, "Wait for new firmware...");
    while (FirmwareManager_Active())
    {
        FirmwareManager_Update();
        UpdateStatusLED();
    }
}

static bool IsUpdateRequested(void)
{
    const struct nvcom_data_t *data_p = NVCom_GetData();
    return data_p->number_of_restarts > 0 && data_p->request_firmware_update;
}

static void ClearUpdateRequest(void)
{
    struct nvcom_data_t *data_p = NVCom_GetData();
    data_p->request_firmware_update = false;
    NVCom_SetData(data_p);
}

static inline bool IsWatchdogRestart(void)
{
    const uint32_t reset_flags = Board_GetResetFlags();

    return (bool)(reset_flags & RCC_CSR_IWDGRSTF);
}

static void UpdateStatusLED(void)
{
    const uint32_t status_led_period_ms = FirmwareManager_DownloadActive() ? 50 : 1000;

    if (SysTime_GetDifference(module.status_led_last_update) >= status_led_period_ms)
    {
        Board_ToggleStatusLED();
        module.status_led_last_update = SysTime_GetSystemTime();
    }
}

static void Reset(void)
{
    ClearUpdateRequest();
    Board_Reset();
}
