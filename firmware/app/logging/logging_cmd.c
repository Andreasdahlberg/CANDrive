/**
 * @file   logging_cmd.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Console commands for the logging module.
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

#include "console.h"
#include "logging.h"
#include "logging_cmd.h"

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

static bool GetName(char **name_p);
static bool GetLevel(int32_t *level_p);
static bool IsLevelArgValid(int32_t arg);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

bool LoggingCmd_SetLevel(void)
{
    bool status = false;

    int32_t level;
    char *name_p;
    if (GetName(&name_p) && GetLevel(&level) && IsLevelArgValid(level))
    {
        logging_logger_t *logger_p = Logging_GetLogger(name_p);
        if (logger_p != NULL)
        {
            Logging_SetLevel(logger_p, (enum logging_level_t)level);
            status = true;
        }
    }
    return status;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static bool GetName(char **name_p)
{
    bool status = false;

    char *arg_p;
    if (Console_GetStringArgument(&arg_p))
    {
        *name_p = arg_p;
        status = true;
    }

    return status;
}

static bool GetLevel(int32_t *level_p)
{
    bool status = false;

    int32_t arg;
    if (Console_GetArgument(&arg) && IsLevelArgValid(arg))
    {
        *level_p = arg;
        status = true;
    }

    return status;
}

static bool IsLevelArgValid(int32_t arg)
{
    return (arg >= 0);
}
