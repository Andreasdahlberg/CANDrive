/**
 * @file   signal.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Signal helper.
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

#include "utility.h"
#include "signal.h"

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

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

bool Signal_IsIDValid(enum signal_id_t id)
{
    return  id < SIGNAL_END;
}

const char *Signal_IDToString(enum signal_id_t id)
{
    switch (id)
    {
        case SIGNAL_CONTROL_RPM1:
            return STRINGIFY(SIGNAL_CONTROL_RPM1);
        case SIGNAL_CONTROL_RPM2:
            return STRINGIFY(SIGNAL_CONTROL_RPM2);
        case SIGNAL_CONTROL_CURRENT1:
            return STRINGIFY(SIGNAL_CONTROL_CURRENT1);
        case SIGNAL_CONTROL_CURRENT2:
            return STRINGIFY(SIGNAL_CONTROL_CURRENT2);
        case SIGNAL_CONTROL_MODE1:
            return STRINGIFY(SIGNAL_CONTROL_MODE1);
        case SIGNAL_CONTROL_MODE2:
            return STRINGIFY(SIGNAL_CONTROL_MODE2);
        default:
            return "INVALID";
    }
}
