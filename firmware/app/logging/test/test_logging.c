/**
 * @file   test_can_interface.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the CANInterface module.
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

#include "logging.h"

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

static uint32_t get_timestamp(void)
{
    return 0;
}

static int Setup(void **state)
{
    Logging_Init(get_timestamp);
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

void test_Logging_Init_Invalid(void **state)
{
    expect_assert_failure(Logging_Init(NULL));
}

void test_Logging_GetLogger_Invalid(void **state)
{
    expect_assert_failure(Logging_GetLogger(NULL));
}

void test_Logging_GetLogger(void **state)
{
    /* Get new loggers */
    logging_logger_t *logger_a_p = Logging_GetLogger("TestLoggerA");
    logging_logger_t *logger_b_p = Logging_GetLogger("TestLoggerB");
    assert_non_null(logger_a_p);
    assert_non_null(logger_b_p);
    assert_ptr_not_equal(logger_a_p, logger_b_p);

    /* Get existing loggers */
    assert_ptr_equal(logger_a_p, Logging_GetLogger("TestLoggerA"));
    assert_ptr_equal(logger_b_p, Logging_GetLogger("TestLoggerB"));
}

void test_Logging_GetLogger_NoAvailableLoggers(void **state)
{
    assert_non_null(Logging_GetLogger("TestLoggerA"));
    assert_non_null(Logging_GetLogger("TestLoggerB"));
    assert_non_null(Logging_GetLogger("TestLoggerC"));
    assert_non_null(Logging_GetLogger("TestLoggerD"));
    assert_null(Logging_GetLogger("TestLoggerE"));
    assert_non_null(Logging_GetLogger("TestLoggerA"));
}

void test_Logging_GetLogger_TruncatedName(void **state)
{
    const char logger_name[] = "verylongloggername";
    logging_logger_t *logger_p = Logging_GetLogger(logger_name);
    assert_non_null(logger_p);

    assert_ptr_equal(logger_p, Logging_GetLogger(logger_name));

    const char truncated_logger_name[] = "verylongloggern";
    assert_ptr_equal(logger_p, Logging_GetLogger(truncated_logger_name));
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_FIFO[] =
    {
        cmocka_unit_test(test_Logging_Init_Invalid),
        cmocka_unit_test_setup(test_Logging_GetLogger_Invalid, Setup),
        cmocka_unit_test_setup(test_Logging_GetLogger, Setup),
        cmocka_unit_test_setup(test_Logging_GetLogger_NoAvailableLoggers, Setup),
        cmocka_unit_test_setup(test_Logging_GetLogger_TruncatedName, Setup),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_FIFO, NULL, NULL);
}
