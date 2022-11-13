/**
 * @file   test_device_monitoring.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the Device Monitoring module.
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
#include <string.h>
#include "memfault/components.h"
#include "logging.h"
#include "device_monitoring.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct logging_logger_t *dummy_logger;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static int Setup(void **state)
{
    will_return_always(Logging_GetLogger, dummy_logger);
    will_return_always(memfault_platform_boot, 0);
    DeviceMonitoring_Init();

    return 0;
}

void MockCallback(void)
{
    function_called();
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

void test_DeviceMonitoring_Init(void **state)
{
    will_return_always(Logging_GetLogger, dummy_logger);
    will_return_always(memfault_platform_boot, 0);
    DeviceMonitoring_Init();
}

void test_DeviceMonitoring_TimerCallback(void **state)
{
    /* Default expectations for 'Transport_Update()'. */
    will_return_always(memfault_packetizer_get_chunk, true);
    will_return_always(ISOTP_Send, true);
    expect_any_always(ISOTP_Send, data_p);
    expect_function_call_any(ISOTP_Proccess);

    /* No callback set. */
    DeviceMonitoring_Update();

    /* Callback set. */
    const uint32_t timer_period_ms = 1000;
    DeviceMonitoring_SetCallback(timer_period_ms, MockCallback);

    will_return(SysTime_GetDifference, timer_period_ms - 1);
    DeviceMonitoring_Update();

    expect_function_call(MockCallback);
    will_return(SysTime_GetDifference, timer_period_ms);
    will_return(SysTime_GetSystemTime, 0);
    DeviceMonitoring_Update();

    /* Callback removed.*/
    DeviceMonitoring_SetCallback(timer_period_ms, NULL);
    DeviceMonitoring_Update();
}

void test_DeviceMonitoring_ResetImminent(void **state)
{
    expect_value(memfault_reboot_tracking_mark_reset_imminent, reboot_reason, kMfltRebootReason_UserReset);
    DeviceMonitoring_ResetImminent(DEV_MON_REBOOT_REAS_USER_RESET);

    expect_value(memfault_reboot_tracking_mark_reset_imminent, reboot_reason, kMfltRebootReason_FirmwareUpdate);
    DeviceMonitoring_ResetImminent(DEV_MON_REBOOT_REAS_FW_UPDATE);

    expect_value(memfault_reboot_tracking_mark_reset_imminent, reboot_reason, kMfltRebootReason_SoftwareReset);
    DeviceMonitoring_ResetImminent(DEV_MON_REBOOT_REAS_SW_RESET);

    expect_value(memfault_reboot_tracking_mark_reset_imminent, reboot_reason, kMfltRebootReason_Unknown);
    DeviceMonitoring_ResetImminent(UINT32_MAX);
}


//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_image[] =
    {
        cmocka_unit_test(test_DeviceMonitoring_Init),
        cmocka_unit_test_setup(test_DeviceMonitoring_TimerCallback, Setup),
        cmocka_unit_test_setup(test_DeviceMonitoring_ResetImminent, Setup),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_image, NULL, NULL);
}
