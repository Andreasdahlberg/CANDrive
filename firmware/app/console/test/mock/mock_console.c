/**
 * @file   mock_console.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for console.
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
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

__attribute__((weak)) void Console_Init(console_write_func_t write_func, console_read_func_t read_func)
{
    assert_non_null(write_func);
    assert_non_null(read_func);
}

__attribute__((weak)) void Console_RegisterCommand(const char *name, console_command_func_t handler)
{
    assert_non_null(handler);
}

__attribute__((weak)) void Console_Process(void)
{
    function_called();
}

__attribute__((weak)) bool Console_GetInt32Argument(int32_t *argument_p)
{
    assert_non_null(argument_p);

    bool status = mock_type(bool);
    if (status)
    {
        *argument_p = mock_type(int32_t);
    }

    return status;
}

__attribute__((weak)) bool Console_GetStringArgument(char **argument_p)
{
    assert_non_null(argument_p);
    return mock_type(bool);
}

__attribute__((weak)) bool Console_GetBoolArgument(bool *argument_p)
{
    assert_non_null(argument_p);
    return mock_type(bool);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
