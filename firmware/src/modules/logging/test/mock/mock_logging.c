/**
 * @file   mock_logging.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for logging.
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
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

__attribute__((weak)) void Logging_Init(logging_time_cb_t time_callback)
{
    assert_non_null(time_callback);
    function_called();
}

__attribute__((weak)) logging_logger_t *Logging_GetLogger(const char *name_p)
{
    mock_type(logging_logger_t *);
}

__attribute__((weak)) void Logging_SetLevel(logging_logger_t *logger_p, enum logging_level_t level)
{
}

__attribute__((weak)) void Logging_Log(const logging_logger_t *logger_p, enum logging_level_t level, const char *file_p, uint32_t line, const char *message_p, ...)
{
#ifdef ENABLE_MOCK_OUTPUT
    printf("%s:%u ", file_p, line);
    va_list args;
    va_start(args, message_p);
    vprintf(message_p, args);
    va_end(args);
    printf("\r\n");
#endif
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

