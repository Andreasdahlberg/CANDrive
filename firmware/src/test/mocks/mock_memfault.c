/**
 * @file   mock_memfault.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for Memfault SDK.
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
#include "memfault/components.h"

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

__attribute__((weak)) int memfault_platform_boot(void)
{
    return mock_type(int);
}

__attribute__((weak)) void memfault_reboot_tracking_mark_reset_imminent(eMemfaultRebootReason reboot_reason,
        const sMfltRebootTrackingRegInfo *reg)
{
    check_expected(reboot_reason);
}

__attribute__((weak)) int memfault_metrics_heartbeat_add(MemfaultMetricId key, int32_t amount)
{
    check_expected(amount);
    return mock_type(int);
}

__attribute__((weak)) int memfault_metrics_heartbeat_timer_start(MemfaultMetricId key)
{
    return mock_type(int);
}

__attribute__((weak)) int memfault_metrics_heartbeat_timer_stop(MemfaultMetricId key)
{
    return mock_type(int);
}

__attribute__((weak)) void memfault_data_export_dump_chunks(void)
{
    function_called();
}

__attribute__((weak)) bool memfault_packetizer_get_chunk(void *buf, size_t *buf_len)
{
    assert_non_null(buf);
    return mock_type(bool);
}

__attribute__((weak)) void memfault_packetizer_abort(void)
{
    function_called();
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
