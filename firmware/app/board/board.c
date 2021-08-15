/**
 * @file   board.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Board support module.
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

#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/rcc.h>
#include "board.h"

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

static void SetupClock(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void Board_Init(void)
{
    SetupClock();
}

uint32_t Board_GetHardwareRevision(void)
{
    return 0;
}

uint32_t Board_GetSoftwareRevision(void)
{
    return 0;
}

struct board_id_t Board_GetId(void)
{
    const uint32_t uid_base_address = 0x1FFFF7E8;

    struct board_id_t id;
    id.offset_0 = MMIO32(uid_base_address);
    id.offset_4 = MMIO32(uid_base_address + 4);
    id.offset_8 = MMIO32(uid_base_address + 8);

    return id;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static void SetupClock(void)
{
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
}
