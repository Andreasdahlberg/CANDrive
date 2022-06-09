/**
 * @file   nvcom.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Non volatile communication module.
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
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/pwr.h>
#include "board.h"
#include "nvcom.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define MAGIC_NUMBER 0xABCD

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct nvcom_internal_data_t
{
    uint16_t magic_number;
    uint16_t reserved_1;
    uint16_t number_of_watchdog_restarts;
    uint16_t reserved_2;
    uint16_t number_of_restarts;
    uint16_t reserved_3;
    uint16_t bootloader_flags;
    uint16_t reserved_4;
    uint16_t reset_flags_high;
    uint16_t reserved_5;
    uint16_t reset_flags_low;
    uint16_t reserved_6;
};
_Static_assert(sizeof(struct nvcom_internal_data_t) <= 40, "Backup registers full");

struct module_t
{
    struct nvcom_data_t data;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct module_t module;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static inline void ClockSetup(void);
static void InitData(void);
static inline bool IsColdRestart(const volatile struct nvcom_internal_data_t *data_p);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void NVCom_Init(void)
{
    module = (__typeof__(module)) {0};

    ClockSetup();
    InitData();
}

struct nvcom_data_t *NVCom_GetData(void)
{
    return &module.data;
}

void NVCom_SetData(const struct nvcom_data_t *data_p)
{
    volatile struct nvcom_internal_data_t *internal_data_p = (volatile struct nvcom_internal_data_t *)(Board_GetBackupMemoryAddress());

    pwr_disable_backup_domain_write_protect();
    internal_data_p->magic_number = MAGIC_NUMBER;
    internal_data_p->reset_flags_high = (uint16_t)(data_p->reset_flags >> 16);
    internal_data_p->reset_flags_low = (uint16_t)(data_p->reset_flags & 0xFFFF);
    internal_data_p->number_of_watchdog_restarts = data_p->number_of_watchdog_restarts;
    internal_data_p->number_of_restarts = data_p->number_of_restarts;
    internal_data_p->bootloader_flags = data_p->request_firmware_update | (data_p->firmware_was_updated << 1);
    pwr_enable_backup_domain_write_protect();
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static inline void ClockSetup(void)
{
    rcc_periph_clock_enable(RCC_PWR);
    rcc_periph_clock_enable(RCC_BKP);
}

static void InitData(void)
{
    volatile struct nvcom_internal_data_t *internal_data_p = (volatile struct nvcom_internal_data_t *)(Board_GetBackupMemoryAddress());

    if (IsColdRestart(internal_data_p))
    {
        module.data.reset_flags = 0x00;
        module.data.number_of_watchdog_restarts = 0;
        module.data.number_of_restarts = 0;
        module.data.request_firmware_update = false;
        module.data.firmware_was_updated = false;
    }
    else
    {
        module.data.reset_flags = (uint32_t)internal_data_p->reset_flags_high << 16 | (uint32_t)internal_data_p->reset_flags_low;
        module.data.number_of_watchdog_restarts = internal_data_p->number_of_watchdog_restarts;
        module.data.number_of_restarts = internal_data_p->number_of_restarts;
        module.data.request_firmware_update = (bool)(internal_data_p->bootloader_flags & (1 << 0));
        module.data.firmware_was_updated = (bool)(internal_data_p->bootloader_flags & (1 << 1));
    }
}

static inline bool IsColdRestart(const volatile struct nvcom_internal_data_t *data_p)
{
    return data_p->magic_number != MAGIC_NUMBER;
}
