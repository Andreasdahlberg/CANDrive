/**
 * @file   test_systime.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the system time module.
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
#include "utility.h"
#include "systime.h"

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
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

void sys_tick_handler(void);

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void ExpectSysTickSetup(void)
{
    expect_value(systick_set_frequency, freq, 1000);
    expect_value(systick_set_frequency, ahb, 72000000);
    will_return(systick_set_frequency, true);
    expect_function_call(systick_interrupt_enable);
    expect_function_call(systick_counter_enable);
}

void AdvanceSystemTime(uint64_t ms)
{
    for (uint64_t i = 0; i < ms; ++i)
    {
        sys_tick_handler();
    }
}

static int Setup(void **state)
{
    ExpectSysTickSetup();
    SysTime_Init();
    return 0;
}

static uint32_t MicrosecondsToTicks(uint32_t microseconds)
{
    const uint32_t ratio = 72000;
    return ratio - ((microseconds * ratio + 500) / 1000);
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_SysTime_Init(void **state)
{
    ExpectSysTickSetup();
    SysTime_Init();
}

static void test_SysTime_GetSystemTime(void **state)
{
    assert_int_equal(SysTime_GetSystemTime(), 0);
    AdvanceSystemTime(100);
    assert_int_equal(SysTime_GetSystemTime(), 100);
}

static void test_SysTime_GetSystemTimeUs(void **state)
{
    const uint32_t ratio = 72000;
    const uint32_t us[] = {0, 1, 500, 999};

    will_return_maybe(systick_get_reload, ratio - 1);

    /* Only microseconds */
    for (size_t i = 0; i < ElementsIn(us); ++i)
    {
        will_return_count(systick_get_countflag, false, 2);
        will_return(systick_get_value, MicrosecondsToTicks(us[i]));
        assert_int_equal(SysTime_GetSystemTimeUs(), us[i]);
    }

    const uint32_t milliseconds = 100;
    AdvanceSystemTime(milliseconds);

    /* Milliseconds and microseconds */
    for (size_t i = 0; i < ElementsIn(us); ++i)
    {
        will_return_count(systick_get_countflag, false, 2);
        will_return(systick_get_value, MicrosecondsToTicks(us[i]));
        assert_int_equal(SysTime_GetSystemTimeUs(), 1000 * milliseconds + us[i]);
    }

    /* Millisecond wrap over */
    will_return_count(systick_get_countflag, true, 2);
    will_return(systick_get_value, MicrosecondsToTicks(999));
    will_return(systick_get_value, MicrosecondsToTicks(1));
    assert_int_equal(SysTime_GetSystemTimeUs(), 1000 * milliseconds + 1);
}

static void test_SysTime_GetSystemTimestamp(void **state)
{
    assert_int_equal(SysTime_GetSystemTimestamp(), 0);
    AdvanceSystemTime(999);
    assert_int_equal(SysTime_GetSystemTimestamp(), 0);
    AdvanceSystemTime(1);
    assert_int_equal(SysTime_GetSystemTimestamp(), 1);
    AdvanceSystemTime(1);
    assert_int_equal(SysTime_GetSystemTimestamp(), 1);
    AdvanceSystemTime(999);
    assert_int_equal(SysTime_GetSystemTimestamp(), 2);
}

void test_SysTime_GetDifference_zero(void **state)
{
    assert_int_equal(SysTime_GetDifference(0), 0);
    AdvanceSystemTime(500);
    assert_int_equal(SysTime_GetDifference(500), 0);
}

void test_SysTime_GetDifference_nonzero(void **state)
{
    AdvanceSystemTime(100);
    assert_int_equal(SysTime_GetDifference(0), 100);
    AdvanceSystemTime(400);
    assert_int_equal(SysTime_GetDifference(100), 400);
}

void test_SysTime_GetDifference_wraparound(void **state)
{
    const uint32_t system_time = (UINT32_MAX - 100);

    /**
     * No real wrap around is performed since it takes to long to call
     * 'sys_tick_handler()' UINT32_MAX times.
     */
    AdvanceSystemTime(99);
    assert_int_equal(SysTime_GetDifference(system_time), 200);
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_SystemTime[] =
    {
        cmocka_unit_test(test_SysTime_Init),
        cmocka_unit_test_setup(test_SysTime_GetSystemTime, Setup),
        cmocka_unit_test_setup(test_SysTime_GetSystemTimeUs, Setup),
        cmocka_unit_test_setup(test_SysTime_GetSystemTimestamp, Setup),
        cmocka_unit_test_setup(test_SysTime_GetDifference_zero, Setup),
        cmocka_unit_test_setup(test_SysTime_GetDifference_nonzero, Setup),
        cmocka_unit_test_setup(test_SysTime_GetDifference_wraparound, Setup)
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_SystemTime, NULL, NULL);
}
