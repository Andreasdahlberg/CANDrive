/**
 * @file   test_filter.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the filter module.
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
#include "filter.h"

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
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

void test_Filter_Init_Invalid(void **state)
{
    expect_assert_failure(Filter_Init(NULL, 0, 0));
}

void test_Filter_Init(void **state)
{
    struct filter_t filter;
    const uint32_t data[] = {0, 1, UINT32_MAX};
    for (size_t i = 0; i < ElementsIn(data); ++i)
    {
        Filter_Init(&filter, data[i], FILTER_ALPHA(1.0));
        assert_int_equal(Filter_Output(&filter), data[i]);
    }
}

void test_Filter_Process(void **state)
{
    struct filter_t filter;
    const double alpha = 0.1;
    Filter_Init(&filter, 0, FILTER_ALPHA(alpha));

    /**
     * The first N datum points in an EMA represent about 86% of the total weight in the
     * calculation where N = (2 / α) − 1.
     */
    const uint32_t number_of_samples = (2 / alpha) - 1;

    for (size_t i = 0; i < number_of_samples; ++i)
    {
        Filter_Process(&filter, 100);
    }

    /* Accept some difference due to rounding */
    assert_in_range(Filter_Output(&filter), 85, 87);

    /* Check if large values are working. */
    Filter_Init(&filter, UINT32_MAX, FILTER_ALPHA(alpha));
    Filter_Process(&filter, UINT32_MAX);
    assert_int_equal(Filter_Output(&filter), UINT32_MAX);
}

void test_Filter_Output_Invalid(void **state)
{
    expect_assert_failure(Filter_Output(NULL));
}

void test_Filter_IsInitialized_Invalid(void **state)
{
    expect_assert_failure(Filter_IsInitialized(NULL));
}

void test_Filter_IsInitialized(void **state)
{
    struct filter_t filter = {0};
    assert_false(Filter_IsInitialized(&filter));

    Filter_Init(&filter, 0, FILTER_ALPHA(0.1));
    assert_true(Filter_IsInitialized(&filter));
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_FIFO[] =
    {
        cmocka_unit_test(test_Filter_Init_Invalid),
        cmocka_unit_test(test_Filter_Init),
        cmocka_unit_test(test_Filter_Process),
        cmocka_unit_test(test_Filter_Output_Invalid),
        cmocka_unit_test(test_Filter_IsInitialized_Invalid),
        cmocka_unit_test(test_Filter_IsInitialized),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_FIFO, NULL, NULL);
}
