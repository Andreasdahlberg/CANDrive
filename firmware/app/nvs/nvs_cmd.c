/**
 * @file   nvs_cmd.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Console commands for the NVS module.
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

#include <assert.h>
#include <stdio.h>
#include "utility.h"
#include "console.h"
#include "nvs.h"
#include "nvs_cmd.h"

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
static bool GetValue(uint32_t *value_p);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

bool NVSCmd_Store(void)
{
    bool status = false;

    uint32_t value;
    char *name_p;
    if (GetName(&name_p) && GetValue(&value))
    {
        status = NVS_Store(name_p, value);
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
    if (Console_GetArgument(&arg_p))
    {
        *name_p = arg_p;
        status = true;
    }

    return status;
}

static bool GetValue(uint32_t *value_p)
{
    bool status = false;

    uint32_t arg;
    if (Console_GetArgument(&arg))
    {
        *value_p = arg;
        status = true;
    }

    return status;
}
