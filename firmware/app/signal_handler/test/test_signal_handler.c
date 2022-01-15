/**
 * @file   test_signal_handler.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the signal handler.
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
#include <stdbool.h>
#include <string.h>
#include "utility.h"
#include "candb.h"
#include "signal_handler.h"
#include "signal.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define WATCHDOG_HANDLE 2

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct logging_logger_t *dummy_logger;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static int Setup(void **state)
{
    will_return(SystemMonitor_GetWatchdogHandle, WATCHDOG_HANDLE);
    will_return_maybe(Logging_GetLogger, dummy_logger);
    SignalHandler_Init();
    return 0;
}

static void SignalHandlerFunc1(struct signal_t *signal_p)
{
    assert_non_null(signal_p);
    function_called();
}

static void SignalHandlerFunc2(struct signal_t *signal_p)
{
    assert_non_null(signal_p);
    function_called();
}

static void SignalHandlerFunc3(struct signal_t *signal_p)
{
    assert_non_null(signal_p);
    function_called();
}

static void SignalHandlerFuncWithSignalCheck(struct signal_t *signal_p)
{
    assert_non_null(signal_p);
    check_expected(*((uint16_t *)signal_p->data_p));
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_SignalHandler_Process(void **state)
{
    /* Expect nothing to happen since no frames are received. */
    expect_value(SystemMonitor_FeedWatchdog, handle, WATCHDOG_HANDLE);
    SignalHandler_Process();

    struct can_frame_t frame = {0};
    frame.size = 8;
    frame.id = CANDB_CONTROLLER_MSG_MOTOR_CONTROL_FRAME_ID;
    SignalHandler_Listener(&frame, NULL);
    /* Expect nothing to happen(frame is discarded) since no signal handlers are registered. */
    expect_function_call(SystemMonitor_ReportActivity);
    expect_value(SystemMonitor_FeedWatchdog, handle, WATCHDOG_HANDLE);
    SignalHandler_Process();

    SignalHandler_RegisterHandler(SIGNAL_CONTROL_RPM1, SignalHandlerFunc1);
    SignalHandler_RegisterHandler(SIGNAL_CONTROL_CURRENT1, SignalHandlerFunc2);
    SignalHandler_RegisterHandler(SIGNAL_CONTROL_MODE1, SignalHandlerFunc3);

    frame.id = 0x00;
    SignalHandler_Listener(&frame, NULL);
    /* Expect nothing to happen since the frame is unsupported. */
    expect_value(SystemMonitor_FeedWatchdog, handle, WATCHDOG_HANDLE);
    SignalHandler_Process();

    frame.id = CANDB_CONTROLLER_MSG_MOTOR_CONTROL_FRAME_ID;
    SignalHandler_Listener(&frame, NULL);
    expect_function_call(SignalHandlerFunc1);
    expect_function_call(SignalHandlerFunc2);
    expect_function_call(SignalHandlerFunc3);
    expect_function_call(SystemMonitor_ReportActivity);
    expect_value(SystemMonitor_FeedWatchdog, handle, WATCHDOG_HANDLE);
    SignalHandler_Process();

    /* Try to fill the frame buffer with unsupported frames. */
    frame.id = 0x00;
    const size_t max_number_of_frames = 5;
    for (size_t i = 0; i < max_number_of_frames; ++i)
    {
        SignalHandler_Listener(&frame, NULL);
    }

    /* Send supported frame, expect it to be handled since the invalid frames are discarded. */
    frame.id = CANDB_CONTROLLER_MSG_MOTOR_CONTROL_FRAME_ID;
    SignalHandler_Listener(&frame, NULL);
    expect_function_call(SignalHandlerFunc1);
    expect_function_call(SignalHandlerFunc2);
    expect_function_call(SignalHandlerFunc3);
    expect_function_call(SystemMonitor_ReportActivity);
    expect_value(SystemMonitor_FeedWatchdog, handle, WATCHDOG_HANDLE);
    SignalHandler_Process();
}

static void test_SignalHandler_Process_FullFIFO(void **state)
{
    SignalHandler_RegisterHandler(SIGNAL_CONTROL_RPM1, SignalHandlerFuncWithSignalCheck);

    struct can_frame_t frame = {0};
    frame.size = 8;
    frame.id = CANDB_CONTROLLER_MSG_MOTOR_CONTROL_FRAME_ID;

    struct candb_controller_msg_motor_control_t msg;
    msg.controller_msg_motor_control_sig_rpm1 = 1;
    candb_controller_msg_motor_control_pack(frame.data, &msg, sizeof(frame.data));

    /* Fill the frame buffer. */
    const size_t max_number_of_frames = 5;
    for (size_t i = 0; i < max_number_of_frames; ++i)
    {
        SignalHandler_Listener(&frame, NULL);
    }

    /* Expect this frame to be discarded. */
    msg.controller_msg_motor_control_sig_rpm1 = 2;
    candb_controller_msg_motor_control_pack(frame.data, &msg, sizeof(frame.data));
    SignalHandler_Listener(&frame, NULL);

    for (size_t i = 0; i < max_number_of_frames; ++i)
    {
        expect_function_call(SystemMonitor_ReportActivity);
        expect_value(SystemMonitor_FeedWatchdog, handle, WATCHDOG_HANDLE);
        expect_value(SignalHandlerFuncWithSignalCheck, *((uint16_t *)signal_p->data_p), 1);
        SignalHandler_Process();
    }
}

static void test_SignalHandler_Process_InvalidFrameSize(void **state)
{
    const uint8_t sizes[] = {0, 7};

    SignalHandler_RegisterHandler(SIGNAL_CONTROL_RPM1, SignalHandlerFunc1);

    for (size_t i = 0; i < ElementsIn(sizes); ++i)
    {
        struct can_frame_t frame;
        frame.size = sizes[i];
        frame.id = CANDB_CONTROLLER_MSG_MOTOR_CONTROL_FRAME_ID;

        SignalHandler_Listener(&frame, NULL);
        /* Expect nothing to happen since the frame is invalid. */
        expect_value(SystemMonitor_FeedWatchdog, handle, WATCHDOG_HANDLE);
        SignalHandler_Process();
    }
}

static void test_SignalHandler_RegisterHandler_Invalid(void **state)
{
    expect_assert_failure(SignalHandler_RegisterHandler(SIGNAL_END, SignalHandlerFunc1));
    expect_assert_failure(SignalHandler_RegisterHandler(SIGNAL_CONTROL_RPM1, NULL));
}

static void test_SignalHandler_RegisterHandler_MaxNumberOfHandlers(void **state)
{
    const size_t max_number_of_handlers = 5;
    for (size_t i = 0; i < max_number_of_handlers; ++i)
    {
        SignalHandler_RegisterHandler(SIGNAL_CONTROL_RPM1, SignalHandlerFunc1);
    }

    expect_assert_failure(SignalHandler_RegisterHandler(SIGNAL_CONTROL_RPM1, SignalHandlerFunc1));
}

static void test_SignalHandler_Listener_Invalid(void **state)
{
    expect_assert_failure(SignalHandler_Listener(NULL, NULL));
}

static void test_SignalHandler_SendMotorStatus_OutOfRange(void **state)
{
    /* Expect no calls to 'CANInterface_Transmit'. */
    assert_false(SignalHandler_SendMotorStatus(20000, 0, 0, 0, 0, 0));
    assert_false(SignalHandler_SendMotorStatus(0, 10000, 0, 0, 0, 0));
    assert_false(SignalHandler_SendMotorStatus(0, 0, 0, 20000, 0, 0));
    assert_false(SignalHandler_SendMotorStatus(0, 0, 0, 0, 10000, 0));
}

static void test_SignalHandler_SendMotorStatus_TransmitFailed(void **state)
{
    will_return(CANInterface_Transmit, false);
    assert_false(SignalHandler_SendMotorStatus(1, 200, 1, 0, 0, 0));
}

static void test_SignalHandler_SendMotorStatus(void **state)
{
    will_return(CANInterface_Transmit, true);
    assert_true(SignalHandler_SendMotorStatus(1, 200, 1, 0, 0, 0));
}

static void test_Signal_IsIDValid(void **state)
{
    for (size_t i = 0; i < SIGNAL_END; ++i)
    {
        assert_true(Signal_IsIDValid((enum signal_id_t)i));
    }
    assert_false(Signal_IsIDValid(SIGNAL_END));
}

static void test_Signal_IDToString(void **state)
{
    char *signal_names[] =
    {
        "SIGNAL_CONTROL_RPM1",
        "SIGNAL_CONTROL_RPM2",
        "SIGNAL_CONTROL_CURRENT1",
        "SIGNAL_CONTROL_CURRENT2",
        "SIGNAL_CONTROL_MODE1",
        "SIGNAL_CONTROL_MODE2"
    };

    for (size_t i = 0; i < ElementsIn(signal_names); ++i)
    {
        assert_string_equal(Signal_IDToString((enum signal_id_t)i), signal_names[i]);
    }
    assert_string_equal(Signal_IDToString(SIGNAL_END), "INVALID");
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_SignalHandler[] =
    {
        cmocka_unit_test_setup(test_SignalHandler_Process, Setup),
        cmocka_unit_test_setup(test_SignalHandler_Process_FullFIFO, Setup),
        cmocka_unit_test_setup(test_SignalHandler_Process_InvalidFrameSize, Setup),
        cmocka_unit_test_setup(test_SignalHandler_RegisterHandler_Invalid, Setup),
        cmocka_unit_test_setup(test_SignalHandler_RegisterHandler_MaxNumberOfHandlers, Setup),
        cmocka_unit_test_setup(test_SignalHandler_Listener_Invalid, Setup),
        cmocka_unit_test_setup(test_SignalHandler_SendMotorStatus_OutOfRange, Setup),
        cmocka_unit_test_setup(test_SignalHandler_SendMotorStatus_TransmitFailed, Setup),
        cmocka_unit_test_setup(test_SignalHandler_SendMotorStatus, Setup)
    };

    const struct CMUnitTest test_Signal[] =
    {
        cmocka_unit_test(test_Signal_IsIDValid),
        cmocka_unit_test(test_Signal_IDToString)
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    int status = cmocka_run_group_tests(test_SignalHandler, NULL, NULL);
    status += cmocka_run_group_tests(test_Signal, NULL, NULL);
    return status;
}
