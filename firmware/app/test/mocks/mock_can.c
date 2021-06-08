/**
 * @file   mock_can.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for can.
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
#include <libopencm3/stm32/can.h>

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

__attribute__((weak)) void can_reset(uint32_t canport)
{
}

__attribute__((weak)) int can_init(uint32_t canport, bool ttcm, bool abom, bool awum, bool nart, bool rflm, bool txfp, uint32_t sjw, uint32_t ts1, uint32_t ts2, uint32_t brp, bool loopback, bool silent)
{
    mock_type(int);
}

__attribute__((weak)) void can_filter_init(uint32_t nr, bool scale_32bit, bool id_list_mode, uint32_t fr1, uint32_t fr2, uint32_t fifo, bool enable)
{
}

__attribute__((weak)) void can_filter_id_mask_16bit_init(uint32_t nr, uint16_t id1, uint16_t mask1, uint16_t id2, uint16_t mask2, uint32_t fifo, bool enable)
{
}

__attribute__((weak)) void can_filter_id_mask_32bit_init(uint32_t nr, uint32_t id, uint32_t mask, uint32_t fifo, bool enable)
{
}

__attribute__((weak)) void can_filter_id_list_16bit_init(uint32_t nr, uint16_t id1, uint16_t id2, uint16_t id3, uint16_t id4, uint32_t fifo, bool enable)
{
}

__attribute__((weak)) void can_filter_id_list_32bit_init(uint32_t nr, uint32_t id1, uint32_t id2, uint32_t fifo, bool enable)
{
}

__attribute__((weak)) void can_enable_irq(uint32_t canport, uint32_t irq)
{
}

__attribute__((weak)) void can_disable_irq(uint32_t canport, uint32_t irq)
{
}

__attribute__((weak)) int can_transmit(uint32_t canport, uint32_t id, bool ext, bool rtr, uint8_t length, uint8_t *data)
{
    mock_type(int);
}

__attribute__((weak)) void can_receive(uint32_t canport, uint8_t fifo, bool release, uint32_t *id, bool *ext, bool *rtr, uint8_t *fmi, uint8_t *length, uint8_t *data, uint16_t *timestamp)
{
}

__attribute__((weak)) void can_fifo_release(uint32_t canport, uint8_t fifo)
{
}

__attribute__((weak)) bool can_available_mailbox(uint32_t canport)
{
    mock_type(bool);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
