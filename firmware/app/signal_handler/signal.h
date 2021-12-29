/**
 * @file   signal.h
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

#ifndef SIGNAL_H_
#define SIGNAL_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "logging.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define Signal_Log(signal, logger) Logging_Debug((logger), "signal: {name: %s, id: %u, data: %u}", Signal_IDToString((signal)->id), (signal)->id, *(uint16_t*)(signal)->data_p)

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

enum signal_id_t
{
    SIGNAL_CONTROL_RPM1 = 0,
    SIGNAL_CONTROL_RPM2,
    SIGNAL_CONTROL_CURRENT1,
    SIGNAL_CONTROL_CURRENT2,
    SIGNAL_CONTROL_MODE1,
    SIGNAL_CONTROL_MODE2,
    /* No new signals after SIGNAL_END!*/
    SIGNAL_END
};

struct signal_t
{
    enum signal_id_t id;
    void *data_p;
};

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

/**
 * Check if the supplied signal ID is valid.
 *
 * @param  id
 * @return True if valid, otherwise false.
 */
bool Signal_IsIDValid(enum signal_id_t id);

/**
 * Get the name of the supplied ID.
 *
 * @param  id
 * @return Name.
 */
const char *Signal_IDToString(enum signal_id_t id);

#endif
