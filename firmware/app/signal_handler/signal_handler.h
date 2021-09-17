/**
 * @file   signal_handler.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Signal handler.
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

#ifndef SIGNAL_HANDLER_H_
#define SIGNAL_HANDLER_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "can_interface.h"
#include "signal.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

typedef void (*signalhandler_handler_cb_t)(struct signal_t *signal_p);

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize the signal handler.
 */
void SignalHandler_Init(void);

/**
 * Receive and store CAN frames for further processing.
 *
 * @param frame_p Pointer to CAN frame.
 */
void SignalHandler_Listener(const struct can_frame_t *frame_p);

/**
 * Process CAN frames into separate signals.
 */
void SignalHandler_Process(void);

/**
 * Register handler for received signals.
 *
 * @param id Signal ID.
 * @param handler_cb Callback function to signal handler.
 */
void SignalHandler_RegisterHandler(enum signal_id_t id, signalhandler_handler_cb_t handler_cb);

#endif
