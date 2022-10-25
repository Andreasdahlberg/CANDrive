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
#include "system_monitor.h"
#include "signal_handler.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define SIGH_LOGGER_NAME "SigH"
#ifndef SIGH_LOGGER_DEBUG_LEVEL
#define SIGH_LOGGER_DEBUG_LEVEL LOGGING_INFO
#endif

#define FRAME_BUFFER_SIZE 5
#define MAX_NUMBER_OF_HANDLERS 6

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
    uint32_t watchdog_handle;
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
    module.watchdog_handle = SystemMonitor_GetWatchdogHandle();
    module.logger = Logging_GetLogger(SIGH_LOGGER_NAME);
    Logging_SetLevel(module.logger, SIGH_LOGGER_DEBUG_LEVEL);
    Logging_Info(module.logger, "Signal handler initialized {wdt_handle: %u}",
                 module.watchdog_handle);
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
                /* Ignore unknown frames */
                break;
        }
    }
    SystemMonitor_FeedWatchdog(module.watchdog_handle);
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

void SignalHandler_Listener(const struct can_frame_t *frame_p, __attribute__((unused)) void *arg_p)
{
    assert(frame_p != NULL);

    if (frame_p->id == CANDB_CONTROLLER_MSG_MOTOR_CONTROL_FRAME_ID)
    {
        bool status = FIFO_Push(&module.frame_fifo, frame_p);
        if (!status)
        {
            Logging_Warning(module.logger, "Buffer full, discard frame: {id: 0x%02x}", frame_p->id);
        }
    }
}

bool SignalHandler_SendMotorStatus(int16_t rpm1, int16_t current1, uint8_t msg_status_1, int16_t rpm2, int16_t current2, uint8_t msg_status_2)
{

    const bool valid_values = (candb_motor_msg_status_motor_status_sig_rpm1_is_in_range(rpm1) &&
                               candb_motor_msg_status_motor_status_sig_current1_is_in_range(current1 ) &&
                               candb_motor_msg_status_motor_status_sig_rpm2_is_in_range(rpm2) &&
                               candb_motor_msg_status_motor_status_sig_current2_is_in_range(current2));

    bool status = true;
    if (valid_values)
    {
        struct candb_motor_msg_status_t msg;
        msg.motor_status_sig_rpm1 = rpm1;
        msg.motor_status_sig_current1 = current1;
        msg.motor_status_sig_rpm2 = rpm2;
        msg.motor_status_sig_current2 = current2;
        msg.motor_status_sig_status1 = msg_status_1;
        msg.motor_status_sig_status2 = msg_status_2;

        uint8_t data[CANDB_MOTOR_MSG_STATUS_LENGTH];
        const int32_t pack_status = candb_motor_msg_status_pack(data, &msg, sizeof(data));
        assert(pack_status != -EINVAL);

        if (!CANInterface_Transmit(CANDB_MOTOR_MSG_STATUS_FRAME_ID, data, CANDB_MOTOR_MSG_STATUS_LENGTH))
        {
            Logging_Warning(module.logger, "Failed to send msg: {id: 0x%02x}", CANDB_MOTOR_MSG_STATUS_FRAME_ID);
            status = false;
        }
    }
    else
    {
        Logging_Warning(module.logger, "Value(s) out of range: {rpm1: %u, current1: %u, rpm2: %u, current2: %u}", rpm1, current1, rpm2, current2);
        status = false;
    }

    return status;
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
        struct signal_t control_signal;

        control_signal.id = SIGNAL_CONTROL_RPM1;
        control_signal.data_p = &msg.motor_control_sig_rpm1;
        DistributeSignal(&control_signal);

        control_signal.id = SIGNAL_CONTROL_RPM2;
        control_signal.data_p = &msg.motor_control_sig_rpm2;
        DistributeSignal(&control_signal);

        control_signal.id = SIGNAL_CONTROL_CURRENT1;
        control_signal.data_p = &msg.motor_control_sig_current1;
        DistributeSignal(&control_signal);

        control_signal.id = SIGNAL_CONTROL_CURRENT2;
        control_signal.data_p = &msg.motor_control_sig_current2;
        DistributeSignal(&control_signal);

        control_signal.id = SIGNAL_CONTROL_MODE1;
        control_signal.data_p = &msg.motor_control_sig_mode1;
        DistributeSignal(&control_signal);

        control_signal.id = SIGNAL_CONTROL_MODE2;
        control_signal.data_p = &msg.motor_control_sig_mode2;
        DistributeSignal(&control_signal);

        SystemMonitor_ReportActivity();
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


