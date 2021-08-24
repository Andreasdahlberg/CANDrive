/**
 * @file   test_console.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the console module.
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
#include "console.h"

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

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static void MockWrite(const char *str)
{
    check_expected(str);
}

static size_t MockRead(char *str)
{
    *str = mock_type(char);
    return mock_type(size_t);
}

static bool MockCommandHandler(void)
{
    return mock_type(bool);
}

static bool MockCommandHandlerWithArgs(void)
{
    bool status;
    int32_t arg_int32;
    bool arg_bool;
    char *arg_string;

    status = Console_GetArgument(&arg_string) &&
             Console_GetArgument(&arg_int32) &&
             Console_GetArgument(&arg_bool);

    check_expected(arg_string);
    check_expected(arg_int32);
    check_expected(arg_bool);

    return status;
}

static int Setup(void **state)
{
    expect_string(MockWrite, str, "\r\n> ");
    Console_Init(MockWrite, MockRead);
    return 0;
}

static void ExpectWrite(char *str)
{
    expect_string(MockWrite, str, str);
}

static void ExpectRead(char *str)
{
    for (size_t i = 0; i < strlen(str); ++i)
    {
        will_return(MockRead, str[i]);
        will_return(MockRead, 1);
    }
}

static void ExpectCommand(char *str)
{
    char cmd[32] = {0};

    ExpectRead(str);

    for (size_t i = 0; i < strlen(str); ++i)
    {
        ExpectWrite("\r> ");
        cmd[i] = str[i];

        ExpectWrite(cmd);
        Console_Process();
    }
}

static void ExpectEndOfCommand(bool status)
{
    char end_of_command[] = "\r";
    ExpectRead(end_of_command);


    if (status)
    {
        ExpectWrite("\r\n[OK]");
    }
    else
    {
        ExpectWrite("\r\n[FAIL]");
    }

    ExpectWrite("\r\n> ");

    Console_Process();
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_Console_Init_Invalid(void **state)
{
    expect_assert_failure(Console_Init(MockWrite, NULL));
    expect_assert_failure(Console_Init(NULL, MockRead));
    expect_assert_failure(Console_Init(NULL, NULL));
}

static void test_Console_Init(void **state)
{
    expect_string(MockWrite, str, "\r\n> ");

    Console_Init(MockWrite, MockRead);
}

static void test_Console_RegisterCommand_Invalid(void **state)
{
    const char name[] = "mock_command";

    /* Invalid parameters */
    expect_assert_failure(Console_RegisterCommand(name, NULL));
    expect_assert_failure(Console_RegisterCommand(NULL, MockCommandHandler));
    expect_assert_failure(Console_RegisterCommand(NULL, NULL));

    /* Too many commands registered */
    const size_t max_number_of_commands = 4;
    for (size_t i = 0; i < max_number_of_commands; ++i)
    {
        Console_RegisterCommand(name, MockCommandHandler);
    }
    expect_assert_failure(Console_RegisterCommand(name, MockCommandHandler));
}

static void test_Console(void **state)
{
    Console_RegisterCommand("mock_command", MockCommandHandler);
    Console_RegisterCommand("mock_command_args", MockCommandHandlerWithArgs);

    ExpectCommand("invalid_command");
    ExpectEndOfCommand(false);

    will_return(MockCommandHandler, true);
    ExpectCommand("mock_command");
    ExpectEndOfCommand(true);

    will_return(MockCommandHandler, false);
    ExpectCommand("mock_command");
    ExpectEndOfCommand(false);

    expect_string(MockCommandHandlerWithArgs, arg_string, "foobar");
    expect_value(MockCommandHandlerWithArgs, arg_int32, 512);
    expect_value(MockCommandHandlerWithArgs, arg_bool, true);
    ExpectCommand("mock_command_args foobar 512 1");
    ExpectEndOfCommand(true);

}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_ADC[] =
    {
        cmocka_unit_test(test_Console_Init_Invalid),
        cmocka_unit_test(test_Console_Init),
        cmocka_unit_test_setup(test_Console_RegisterCommand_Invalid, Setup),
        cmocka_unit_test_setup(test_Console, Setup),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_ADC, NULL, NULL);
}
