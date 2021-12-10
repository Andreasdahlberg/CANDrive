/**
 * @file   config.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Implementation of device configuration module.
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

#include "nvs.h"
#include "config.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct config_t
{
    uint32_t number_of_motors;
    uint32_t counts_per_rev;
    uint32_t no_load_rpm;
    uint32_t no_load_current;
    uint32_t stall_current;
    uint32_t max_current;
};

struct module_t
{
    struct config_t config;
    bool valid_config;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct module_t module;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void Config_Init(void)
{
    module = (__typeof__(module)) {0};
    module.valid_config = NVS_Retrieve("number_of_motors", &module.config.number_of_motors) &&
                          NVS_Retrieve("counts_per_rev", &module.config.counts_per_rev) &&
                          NVS_Retrieve("no_load_rpm", &module.config.no_load_rpm) &&
                          NVS_Retrieve("no_load_current", &module.config.no_load_current) &&
                          NVS_Retrieve("stall_current", &module.config.stall_current) &&
                          NVS_Retrieve("max_current", &module.config.max_current);

    /* Clear all parameters if the configuration is invalid. */
    if (!Config_IsValid())
    {
        module = (__typeof__(module)) {0};
    }
}

bool Config_IsValid(void)
{
    return module.valid_config;
}

uint32_t Config_GetNumberOfMotors(void)
{
    return module.config.number_of_motors;
}

uint32_t Config_GetCountsPerRev(void)
{
    return module.config.counts_per_rev;
}

uint32_t Config_GetNoLoadRpm(void)
{
    return module.config.no_load_rpm;
}

uint32_t Config_GetNoLoadCurrent(void)
{
    return module.config.no_load_current;
}

uint32_t Config_GetStallCurrent(void)
{
    return module.config.stall_current;
}

uint32_t Config_GetMaxCurrent(void)
{
    return module.config.max_current;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
