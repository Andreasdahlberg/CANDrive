/**
 * @file   mock_nvic.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for nvic.
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
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <libopencm3/cm3/nvic.h>

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

__attribute__((weak)) void nvic_enable_irq(uint8_t irqn)
{
}

__attribute__((weak)) void nvic_disable_irq(uint8_t irqn)
{
}

__attribute__((weak)) uint8_t nvic_get_pending_irq(uint8_t irqn)
{
    mock_type(uint8_t);
}

__attribute__((weak)) void nvic_set_pending_irq(uint8_t irqn)
{
}

__attribute__((weak)) void nvic_clear_pending_irq(uint8_t irqn)
{
}

__attribute__((weak)) uint8_t nvic_get_irq_enabled(uint8_t irqn)
{
    mock_type(uint8_t);
}

__attribute__((weak)) void nvic_set_priority(uint8_t irqn, uint8_t priority)
{
}

__attribute__((weak)) void reset_handler(void)
{
}

__attribute__((weak)) void nmi_handler(void)
{
}

__attribute__((weak)) void hard_fault_handler(void)
{
}

__attribute__((weak)) void sv_call_handler(void)
{
}

__attribute__((weak)) void pend_sv_handler(void)
{
}

__attribute__((weak)) void sys_tick_handler(void)
{
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
