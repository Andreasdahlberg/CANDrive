/**
 * @file   can_interface.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  CAN-Bus interface.
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

#ifndef CAN_INTERFACE_H_
#define CAN_INTERFACE_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct can_frame_t
{
    uint32_t id;
    uint8_t size;
    uint8_t data[8];
};

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

typedef void (*caninterface_listener_cb_t)(const struct can_frame_t *frame_p, void *arg_p);

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize the CAN interface.
 *
 * Note: It's required to setup acceptance filters before any CAN-frames can be received.
 *       See 'CANInterface_AddFilter'.
 */
void CANInterface_Init(void);

/**
 * Transmit a CAN-frame
 *
 * @param  id ID
 * @param  data_p Pointer to data.
 * @param  size Size of data, max 8.
 *
 * @return True if frame was sent/queued, otherwise false.
 */
bool CANInterface_Transmit(uint32_t id, void *data_p, size_t size);

/**
 * Register a listener.
 *
 * The registered callback will be called when a CAN-frame is received.
 * Note that the callback is called from an ISR.
 *
 * @param listener_cb Callback.
 * @param arg_p Argument passed to the callback.
 */
void CANInterface_RegisterListener(caninterface_listener_cb_t listener_cb, void *arg_p);

/**
 * Add an acceptance filter to the CAN interface.
 *
 * Up to 28 filters can be added.
 *
 * @param id CAN-frame ID.
 * @param mask CAN-frame ID bit mask.
 */
void CANInterface_AddFilter(uint16_t id, uint16_t mask);

#endif
