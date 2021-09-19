/**
 * @file   signal_handler.c
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

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stddef.h>
#include <assert.h>
#include "utility.h"
#include "logging.h"
#include "candb.h"
#include "fifo.h"
#include "signal_handler.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define SIGH_LOGGER_NAME "SigH"
#ifndef SIGH_LOGGER_DEBUG_LEVEL
#define SIGH_LOGGER_DEBUG_LEVEL LOGGING_DEBUG
#endif

#define FRAME_BUFFER_SIZE 5
#define MAX_NUMBER_OF_HANDLERS 5

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct handler_t
{
    enum signal_id_t id;
    signalhandler_handler_cb_t callback;
};

struct module_t
{
    logging_logger_t *logger;
    struct can_frame_t frame_buffer[FRAME_BUFFER_SIZE];
    struct fifo_t frame_fifo;
    struct handler_t handlers[MAX_NUMBER_OF_HANDLERS];
    size_t number_of_handlers;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct module_t module;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static void HandleMotorControlFrame(const struct can_frame_t *frame_p);
static void DistributeSignal(struct signal_t *signal_p);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void SignalHandler_Init(void)
{
    module = (__typeof__(module)) {0};
    module.frame_fifo = FIFO_New(module.frame_buffer);
    module.logger = Logging_GetLogger(SIGH_LOGGER_NAME);
    Logging_SetLevel(module.logger, SIGH_LOGGER_DEBUG_LEVEL);
    Logging_Info(module.logger, "Signal handler initialized");
}

void SignalHandler_Process(void)
{
    struct can_frame_t frame;
    bool status = FIFO_Pop(&module.frame_fifo, &frame);
    if (status)
    {
        Logging_Debug(module.logger, "Process: {id: 0x%02x}", frame.id);

        switch (frame.id)
        {
            case CANDB_CONTROLLER_MSG_MOTOR_CONTROL_FRAME_ID:
                HandleMotorControlFrame(&frame);
                break;

            default:
                Logging_Warning(module.logger, "Unknown: {id: 0x%02x}", frame.id);
                break;
        }
    }
}

void SignalHandler_RegisterHandler(enum signal_id_t id, signalhandler_handler_cb_t handler_cb)
{
    assert(Signal_IsIDValid(id));
    assert(handler_cb != NULL);
    assert(module.number_of_handlers < MAX_NUMBER_OF_HANDLERS);

    module.handlers[module.number_of_handlers].id = id;
    module.handlers[module.number_of_handlers].callback = handler_cb;
    ++module.number_of_handlers;

    Logging_Info(module.logger, "New handler registered: 0x%x", (uintptr_t)handler_cb);
}

void SignalHandler_Listener(const struct can_frame_t *frame_p)
{
    assert(frame_p != NULL);

    bool status = FIFO_Push(&module.frame_fifo, frame_p);
    if (!status)
    {
        Logging_Warning(module.logger, "Buffer full, discard frame: {id: 0x%02x}", frame_p->id);
    }
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static void HandleMotorControlFrame(const struct can_frame_t *frame_p)
{
    Logging_Debug(module.logger, "Unpack: {id: 0x%02x, name: %s}", frame_p->id, STRINGIFY(CANDB_CONTROLLER_MSG_MOTOR_CONTROL_FRAME_ID));

    struct candb_controller_msg_motor_control_t msg;
    const int32_t status = candb_controller_msg_motor_control_unpack(&msg, frame_p->data, frame_p->size);
    if (status != -EINVAL)
    {
        struct signal_t signal;

        signal.id = SIGNAL_CONTROL_RPM1;
        signal.data_p = &msg.controller_msg_motor_control_sig_rpm1;
        DistributeSignal(&signal);

        signal.id = SIGNAL_CONTROL_RPM2;
        signal.data_p = &msg.controller_msg_motor_control_sig_rpm2;
        DistributeSignal(&signal);

        signal.id = SIGNAL_CONTROL_CURRENT1;
        signal.data_p = &msg.controller_msg_motor_control_sig_current1;
        DistributeSignal(&signal);

        signal.id = SIGNAL_CONTROL_CURRENT2;
        signal.data_p = &msg.controller_msg_motor_control_sig_current2;
        DistributeSignal(&signal);
    }
    else
    {
        Logging_Error(module.logger, "Invalid frame: {id: 0x%02x, size: %u}", frame_p->id, frame_p->size);
    }
}

static void DistributeSignal(struct signal_t *signal_p)
{
    for (size_t i = 0; i < module.number_of_handlers; ++i)
    {
        if (module.handlers[i].id == signal_p->id)
        {
            module.handlers[i].callback(signal_p);
        }
    }
}


