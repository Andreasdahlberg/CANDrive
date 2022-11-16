/**
 * @file   mock_device_monitoring.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for device_monitoring.
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
#include <stdbool.h>
#include "device_monitoring.h"
#include "device_monitoring_cmd.h"

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

__attribute__((weak)) void DeviceMonitoring_Init(void)
{

}

__attribute__((weak)) void DeviceMonitoring_Update(void)
{

}

__attribute__((weak)) void DeviceMonitoring_SetCallback(uint32_t period_sec, device_monitoring_timer_cb_t timer_callback)
{

}

__attribute__((weak)) void DeviceMonitoring_ResetImminent(enum device_monitoring_reboot_reason reason)
{
    check_expected(reason);
}

__attribute__((weak)) void DeviceMonitoring_Count(enum device_monitoring_metric_id id, int32_t amount)
{
    check_expected(id);
    check_expected(amount);
}

__attribute__((weak)) void DeviceMonitoring_StartTimer(enum device_monitoring_metric_id id)
{

}

__attribute__((weak)) void DeviceMonitoring_StopTimer(enum device_monitoring_metric_id id)
{

}

__attribute__((weak)) bool DeviceMonitoringCmd_DumpData(void)
{
    return mock_type(bool);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
