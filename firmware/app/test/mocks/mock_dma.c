/**
 * @file   mock_dma.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for dma.
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
#include <libopencm3/stm32/dma.h>

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

__attribute__((weak)) void dma_channel_reset(uint32_t dma, uint8_t channel)
{
    check_expected(dma);
    check_expected(channel);
}

__attribute__((weak)) void dma_clear_interrupt_flags(uint32_t dma, uint8_t channel, uint32_t interrupts)
{
}

__attribute__((weak)) bool dma_get_interrupt_flag(uint32_t dma, uint8_t channel, uint32_t interrupts)
{
    mock_type(bool);
}

__attribute__((weak)) void dma_enable_mem2mem_mode(uint32_t dma, uint8_t channel)
{
}

__attribute__((weak)) void dma_set_priority(uint32_t dma, uint8_t channel, uint32_t prio)
{
}

__attribute__((weak)) void dma_set_memory_size(uint32_t dma, uint8_t channel, uint32_t mem_size)
{
}

__attribute__((weak)) void dma_set_peripheral_size(uint32_t dma, uint8_t channel, uint32_t peripheral_size)
{
}

__attribute__((weak)) void dma_enable_memory_increment_mode(uint32_t dma, uint8_t channel)
{
    check_expected(dma);
    check_expected(channel);
}

__attribute__((weak)) void dma_disable_memory_increment_mode(uint32_t dma, uint8_t channel)
{
}

__attribute__((weak)) void dma_enable_peripheral_increment_mode(uint32_t dma, uint8_t channel)
{
}

__attribute__((weak)) void dma_disable_peripheral_increment_mode(uint32_t dma, uint8_t channel)
{
}

__attribute__((weak)) void dma_enable_circular_mode(uint32_t dma, uint8_t channel)
{
    check_expected(dma);
    check_expected(channel);
}

__attribute__((weak)) void dma_set_read_from_peripheral(uint32_t dma, uint8_t channel)
{
    check_expected(dma);
    check_expected(channel);
}

__attribute__((weak)) void dma_set_read_from_memory(uint32_t dma, uint8_t channel)
{
}

__attribute__((weak)) void dma_enable_transfer_error_interrupt(uint32_t dma, uint8_t channel)
{
}

__attribute__((weak)) void dma_disable_transfer_error_interrupt(uint32_t dma, uint8_t channel)
{
}

__attribute__((weak)) void dma_enable_half_transfer_interrupt(uint32_t dma, uint8_t channel)
{
}

__attribute__((weak)) void dma_disable_half_transfer_interrupt(uint32_t dma, uint8_t channel)
{
}

__attribute__((weak)) void dma_enable_transfer_complete_interrupt(uint32_t dma, uint8_t channel)
{
    check_expected(dma);
    check_expected(channel);
}

__attribute__((weak)) void dma_disable_transfer_complete_interrupt(uint32_t dma, uint8_t channel)
{
}

__attribute__((weak)) void dma_enable_channel(uint32_t dma, uint8_t channel)
{
    check_expected(dma);
    check_expected(channel);
}

__attribute__((weak)) void dma_disable_channel(uint32_t dma, uint8_t channel)
{
}

__attribute__((weak)) void dma_set_peripheral_address(uint32_t dma, uint8_t channel, uint32_t address)
{
}

__attribute__((weak)) void dma_set_memory_address(uint32_t dma, uint8_t channel, uint32_t address)
{
    check_expected(dma);
    check_expected(channel);
}

__attribute__((weak)) uint16_t dma_get_number_of_data(uint32_t dma, uint8_t channel)
{
    mock_type(uint16_t);
}

__attribute__((weak)) void dma_set_number_of_data(uint32_t dma, uint8_t channel, uint16_t number)
{
    check_expected(dma);
    check_expected(channel);
    check_expected(number);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

