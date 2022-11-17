/**
 * @file   mock_isotp.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for isotp.
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
#include <string.h>
#include "isotp.h"

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

__attribute__((weak)) void ISOTP_Bind(struct isotp_ctx_t *ctx_p, void *rx_buffer_p, size_t rx_buffer_size, void *tx_buffer_p, size_t tx_buffer_size, uint16_t rx_id, uint16_t tx_id, isotp_status_callback_t rx_callback_fp, isotp_status_callback_t tx_callback_fp)
{
    assert_non_null(ctx_p);
}

__attribute__((weak)) void ISOTP_SetSeparationTime(struct isotp_ctx_t *ctx_p, uint8_t separation_time)
{
    assert_non_null(ctx_p);
    check_expected(separation_time);
}

__attribute__((weak)) void ISOTP_Proccess(struct isotp_ctx_t *ctx_p)
{
    assert_non_null(ctx_p);
    function_called();
}

__attribute__((weak)) size_t ISOTP_Receive(struct isotp_ctx_t *ctx_p, void *destination_p, size_t length)
{
    assert_non_null(ctx_p);
    size_t number_of_bytes = mock_type(size_t);
    memcpy(destination_p, mock_ptr_type(uint8_t *), number_of_bytes);
    return number_of_bytes;
}

__attribute__((weak)) bool ISOTP_Send(struct isotp_ctx_t *ctx_p, const void *data_p, size_t length)
{
    assert_non_null(ctx_p);
    check_expected(data_p);
    return mock_type(bool);
}

__attribute__((weak)) bool ISOTP_IsSending(const struct isotp_ctx_t *ctx_p)
{
    assert_non_null(ctx_p);
    return mock_type(bool);
}

__attribute__((weak)) void ISOTP_ProccessTx(struct isotp_ctx_t *ctx_p)
{
    assert_non_null(ctx_p);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
