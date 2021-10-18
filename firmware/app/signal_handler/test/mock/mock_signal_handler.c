/**
 * @file   mock_signal_handler.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for signal_handler.
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
#include "signal_handler.h"

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

__attribute__((weak)) void SignalHandler_Init(void)
{
    function_called();
}

__attribute__((weak)) void SignalHandler_Listener(const struct can_frame_t *frame_p)
{
    assert_non_null(frame_p);
    function_called();
}

__attribute__((weak)) void SignalHandler_Process(void)
{
    function_called();
}

__attribute__((weak)) void SignalHandler_RegisterHandler(enum signal_id_t id, signalhandler_handler_cb_t handler_cb)
{
    check_expected(id);
    assert_non_null(handler_cb);
}

__attribute__((weak)) bool SignalHandler_SendMotorStatus(int16_t rpm1, int16_t current1, int16_t rpm2, int16_t current2, uint8_t msg_status)
{
    check_expected(rpm1);
    check_expected(current1);
    check_expected(rpm2);
    check_expected(current2);
    check_expected(msg_status);

    return mock_type(bool);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
