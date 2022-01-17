/**
 * @file   systime.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  System timer module.
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

#include <libopencm3/cm3/systick.h>
#include "systime.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct module_t
{
    volatile uint32_t system_time;
    volatile uint32_t system_timestamp;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

struct module_t module;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

void SetupSysTick(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void SysTime_Init(void)
{
    module = (__typeof__(module)) {0};

    SetupSysTick();
}

uint32_t SysTime_GetSystemTime(void)
{
    return module.system_time;
}

uint32_t SysTime_GetSystemTimeUs(void)
{
    /* Reset the count flag by reading the SysTick control and status register. */
    systick_get_countflag();
    uint32_t system_time = module.system_time;

    const uint32_t ratio = systick_get_reload() + 1; /* 72000 */
    uint32_t counter = systick_get_value();

    if (systick_get_countflag())
    {
        system_time = module.system_time;
        counter = systick_get_value();
    }
    return (system_time * 1000) + ((counter * 1000 + (ratio / 2)) / ratio);
}

uint32_t SysTime_GetSystemTimestamp(void)
{
    return module.system_timestamp;
}

uint32_t SysTime_GetDifference(uint32_t system_time)
{
    uint32_t difference;
    const uint32_t current_system_time = SysTime_GetSystemTime();

    /* Check for overflow. */
    if (system_time > current_system_time)
    {
        difference = (UINT32_MAX - system_time) + current_system_time + 1;
    }
    else
    {
        difference = current_system_time - system_time;
    }
    return difference;
}

//////////////////////////////////////////////////////////////////////////
//ISR
//////////////////////////////////////////////////////////////////////////

void sys_tick_handler(void)
{
    ++module.system_time;

    if (module.system_time % 1000 == 0)
    {
        ++module.system_timestamp;
    }
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void SetupSysTick(void)
{
    const uint32_t tick_frequency = 1000;
    const uint32_t ahb_frequency = 72000000;

    systick_set_frequency(tick_frequency, ahb_frequency);
    systick_interrupt_enable();
    systick_counter_enable();
}
