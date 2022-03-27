/**
 * @file   mock_systick.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for systick.
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

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <libopencm3/cm3/systick.h>

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

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

__attribute__((weak)) void systick_set_reload(uint32_t value)
{
}

__attribute__((weak)) bool systick_set_frequency(uint32_t freq, uint32_t ahb)
{
    check_expected(freq);
    check_expected(ahb);
    mock_type(bool);
}

__attribute__((weak)) uint32_t systick_get_reload(void)
{
    mock_type(uint32_t);
}

__attribute__((weak)) uint32_t systick_get_value(void)
{
    mock_type(uint32_t);
}

__attribute__((weak)) void systick_set_clocksource(uint8_t clocksource)
{
}

__attribute__((weak)) void systick_interrupt_enable(void)
{
    function_called();
}

__attribute__((weak)) void systick_interrupt_disable(void)
{
}

__attribute__((weak)) void systick_counter_enable(void)
{
    function_called();
}

__attribute__((weak)) void systick_counter_disable(void)
{
}

__attribute__((weak)) uint8_t systick_get_countflag(void)
{
    mock_type(uint8_t);
}

__attribute__((weak)) void systick_clear(void)
{
}

__attribute__((weak)) uint32_t systick_get_calib(void)
{
    mock_type(uint32_t);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

