/**
 * @file   test_system_monitor.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the system monitor module.
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
#include "utility.h"
#include "logging.h"
#include <libopencm3/stm32/iwdg.h>
#include <libopencm3/stm32/rcc.h>
#include "system_monitor.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define MAX_NUMBER_OF_HANDLERS 10

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct logging_logger_t *dummy_logger;

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//MOCKS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static void NormalReset(void)
{
    will_return(Logging_GetLogger, dummy_logger);
    will_return_count(Board_GetResetFlags, 0, 2);
    expect_any(iwdg_set_period_ms, period);
    expect_function_call(iwdg_start);
}

static void WatchdogReset(bool expect_assert_failure)
{
    will_return(Logging_GetLogger, dummy_logger);
    will_return_count(Board_GetResetFlags, RCC_CSR_IWDGRSTF, 2);
    if (!expect_assert_failure)
    {
        expect_any(iwdg_set_period_ms, period);
        expect_function_call(iwdg_start);
    }
}

static int Setup(void **state)
{
    NormalReset();
    SystemMonitor_Init();
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_SystemMonitor_Init(void **state)
{
    /* Cold/normal start */
    NormalReset();
    SystemMonitor_Init();

    /* First watchdog reset */
    WatchdogReset(false);
    SystemMonitor_Init();

    /* Second watchdog reset */
    WatchdogReset(false);
    SystemMonitor_Init();

    /* Third watchdog reset, expect assert due to restart loop.*/
    WatchdogReset(true);
    expect_assert_failure(SystemMonitor_Init());

    /**
     * Trigger another watchdog reset to make sure that the reset counter is
     * not cleared.
     */
    WatchdogReset(true);
    expect_assert_failure(SystemMonitor_Init());

    /* Normal restart */
    NormalReset();
    SystemMonitor_Init();

    /**
     * Watchdog reset, expect no failed assert since the normal restart should
     * clear watchdog restart counter.
     */
    WatchdogReset(false);
    SystemMonitor_Init();
}

static void test_SystemMonitor_GetWatchdogHandle_Invalid(void **state)
{
    const uint32_t max_number_of_watchdog_handles = 32;
    for (size_t i = 0; i < max_number_of_watchdog_handles; ++i)
    {
        SystemMonitor_GetWatchdogHandle();
    }

    expect_assert_failure(SystemMonitor_GetWatchdogHandle());
}

static void test_SystemMonitor_GetWatchdogHandle(void **state)
{
    const size_t max_number_of_watchdog_handles = 32;
    uintmax_t handles[max_number_of_watchdog_handles];
    memset(handles, 0xFF, sizeof(handles));

    for (size_t i = 0; i < ElementsIn(handles); ++i)
    {
        const uint32_t handle = SystemMonitor_GetWatchdogHandle();

        assert_not_in_set(handle, handles, ElementsIn(handles));
        handles[i] = handle;
    }
}

static void test_SystemMonitor_FeedWatchdog_Invalid(void **state)
{
    expect_assert_failure(SystemMonitor_FeedWatchdog(0));

    const uint32_t number_of_watchdog_handles = 16;
    for (size_t i = 0; i < number_of_watchdog_handles; ++i)
    {
        SystemMonitor_GetWatchdogHandle();
    }

    expect_assert_failure(SystemMonitor_FeedWatchdog(16));
    expect_assert_failure(SystemMonitor_FeedWatchdog(UINT32_MAX));
}

static void test_SystemMonitor_Update(void **state)
{
    uint32_t handles[3];
    for (size_t i = 0; i < ElementsIn(handles); ++i)
    {
        handles[i] =  SystemMonitor_GetWatchdogHandle();
    }

    /* Expect watchdog reset since the watchdog is feed when getting the handles*/
    expect_function_call(iwdg_reset);
    SystemMonitor_Update();

    SystemMonitor_Update();

    SystemMonitor_FeedWatchdog(handles[0]);
    SystemMonitor_FeedWatchdog(handles[1]);
    SystemMonitor_Update();

    SystemMonitor_FeedWatchdog(handles[0]);
    SystemMonitor_Update();

    SystemMonitor_FeedWatchdog(handles[2]);
    expect_function_call(iwdg_reset);
    SystemMonitor_Update();

    SystemMonitor_Update();
}

static void test_SystemMonitor_ControlActivity(void **state)
{
    SystemMonitor_GetWatchdogHandle();

    assert_int_equal(SystemMonitor_GetState(), SYSTEM_MONITOR_INACTIVE);

    will_return(SysTime_GetSystemTime, 0);
    SystemMonitor_ReportControlActivity();
    assert_int_equal(SystemMonitor_GetState(), SYSTEM_MONITOR_ACTIVE);

    expect_function_call(iwdg_reset);
    will_return(SysTime_GetDifference, 200);
    SystemMonitor_Update();
    assert_int_equal(SystemMonitor_GetState(), SYSTEM_MONITOR_ACTIVE);

    will_return(SysTime_GetDifference, 201);
    SystemMonitor_Update();
    assert_int_equal(SystemMonitor_GetState(), SYSTEM_MONITOR_INACTIVE);
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_system_monitor[] =
    {
        cmocka_unit_test(test_SystemMonitor_Init),
        cmocka_unit_test_setup(test_SystemMonitor_Init, Setup),
        cmocka_unit_test_setup(test_SystemMonitor_GetWatchdogHandle_Invalid, Setup),
        cmocka_unit_test_setup(test_SystemMonitor_GetWatchdogHandle, Setup),
        cmocka_unit_test_setup(test_SystemMonitor_FeedWatchdog_Invalid, Setup),
        cmocka_unit_test_setup(test_SystemMonitor_Update, Setup),
        cmocka_unit_test_setup(test_SystemMonitor_ControlActivity, Setup),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_system_monitor, NULL, NULL);
}
