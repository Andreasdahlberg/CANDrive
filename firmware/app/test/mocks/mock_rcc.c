/**
 * @file   mock_rcc.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for rcc.
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
#include <libopencm3/stm32/rcc.h>

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

__attribute__((weak)) void rcc_peripheral_enable_clock(volatile uint32_t *reg, uint32_t en)
{

}

__attribute__((weak)) void rcc_peripheral_disable_clock(volatile uint32_t *reg, uint32_t en)
{

}

__attribute__((weak)) void rcc_peripheral_reset(volatile uint32_t *reg, uint32_t reset)
{

}

__attribute__((weak)) void rcc_peripheral_clear_reset(volatile uint32_t *reg, uint32_t clear_reset)
{

}

__attribute__((weak)) void rcc_periph_clock_enable(enum rcc_periph_clken clken)
{

}

__attribute__((weak)) void rcc_periph_clock_disable(enum rcc_periph_clken clken)
{

}

__attribute__((weak)) void rcc_periph_reset_pulse(enum rcc_periph_rst rst)
{

}

__attribute__((weak)) void rcc_periph_reset_hold(enum rcc_periph_rst rst)
{

}

__attribute__((weak)) void rcc_periph_reset_release(enum rcc_periph_rst rst)
{

}

__attribute__((weak)) void rcc_set_mco(uint32_t mcosrc)
{

}

__attribute__((weak)) void rcc_osc_bypass_enable(enum rcc_osc osc)
{

}

__attribute__((weak)) void rcc_osc_bypass_disable(enum rcc_osc osc)
{

}

__attribute__((weak)) bool rcc_is_osc_ready(enum rcc_osc osc)
{

}

__attribute__((weak)) void rcc_wait_for_osc_ready(enum rcc_osc osc)
{

}

__attribute__((weak)) uint16_t rcc_get_div_from_hpre(uint8_t div_val)
{
    mock_type(uint16_t);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
