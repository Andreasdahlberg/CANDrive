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
#include "serial.h"
#include "systime.h"
#include "logging.h"
#include "image.h"
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
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct module_t module;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static inline void ClockSetup(void);
static inline void StartApplication(const uintptr_t *start_p) __attribute__((noreturn));
static inline void PrepareForApplication(const struct image_header_t *header_p);
static void JumpToApplication(void *pc, void *sp) __attribute__((naked, noreturn));

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void Bootloader_Init(void)
{
    ClockSetup();
    SysTime_Init();
    Serial_Init(BAUD_RATE);
    Logging_Init(SysTime_GetSystemTime);
    Image_Init();

    module.logger = Logging_GetLogger(BOOTLOADER_LOGGER_NAME);
    Logging_SetLevel(module.logger, BOOTLOADER_LOGGER_DEBUG_LEVEL);
    Logging_Info(module.logger, "Bootloader ready: {sha: %s}", GIT_DESC);
}

void Bootloader_Start(void)
{
    StartApplication(&__approm_start__);

    while (1)
    {
        /* Should never reach this. */
    }
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static inline void ClockSetup(void)
{
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
}

static inline void StartApplication(const uintptr_t *start_p)
{
    const struct image_header_t *header_p = Image_GetHeader(start_p);
    if (header_p != NULL && Image_IsValid(start_p))
    {
        PrepareForApplication(header_p);

        Logging_Info(module.logger, "image: {version: %s, sha: %s, crc: %u, size: %u}",
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
        Logging_Error(module.logger, "Invalid firmware, Wait for firmware download...");
        while (1)
        {
            /* Wait for new firmware here */
        }
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
