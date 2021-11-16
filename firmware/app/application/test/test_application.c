/**
 * @file   test_application.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the application module.
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
#include "signal_handler.h"
#include "system_monitor.h"
#include "motor_controller.h"
#include "application.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define MAX_NUMBER_OF_HANDLERS 10

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct signal_handler_t
{
    enum signal_id_t id;
    signalhandler_handler_cb_t callback;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct logging_logger_t *dummy_logger;
static struct signal_handler_t signal_handlers[MAX_NUMBER_OF_HANDLERS];
static size_t number_of_handlers;

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//MOCKS
//////////////////////////////////////////////////////////////////////////

void SignalHandler_RegisterHandler(enum signal_id_t id, signalhandler_handler_cb_t handler_cb)
{
    check_expected(id);
    assert_non_null(handler_cb);

    signal_handlers[number_of_handlers].id = id;
    signal_handlers[number_of_handlers].callback = handler_cb;
    ++number_of_handlers;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static int Setup(void **state)
{
    number_of_handlers = 0;

    expect_function_call(SysTime_Init);
    expect_any(Serial_Init, baud_rate);
    expect_function_call(Logging_Init);
    expect_function_call(SystemMonitor_Init);
    will_return(Board_GetNVSAddress, 0x801F800);
    will_return(Board_GetNumberOfPagesInNVS, 2);
    expect_function_call(NVS_Init);
    expect_function_call(CANInterface_Init);
    expect_function_call(ADC_Init);
    expect_function_call(MotorController_Init);
    expect_function_call(SignalHandler_Init);
    will_return(Logging_GetLogger, dummy_logger);
    expect_function_call(CANInterface_RegisterListener);
    expect_any(CANInterface_AddFilter, id);
    expect_any(CANInterface_AddFilter, mask);
    expect_any_always(SignalHandler_RegisterHandler, id);
    will_return_maybe(Board_GetResetFlags, 0);
    will_return_maybe(Board_GetHardwareRevision, 1);
    will_return_maybe(Board_GetSoftwareRevision, 2);
    expect_any(MotorController_SetRPM, index);
    expect_any(MotorController_SetRPM, rpm);
    expect_any(MotorController_SetCurrent, index);
    expect_any(MotorController_SetCurrent, current);

    Application_Init();
    return 0;
}

signalhandler_handler_cb_t GetCallback(enum signal_id_t id)
{
    signalhandler_handler_cb_t callback = NULL;

    for (size_t i = 0; i < number_of_handlers; ++i)
    {
        if (signal_handlers[i].id == id)
        {
            callback = signal_handlers[i].callback;
        }
    }

    assert_non_null(callback);
    return  callback;
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_Application_Init(void **state)
{
    number_of_handlers = 0;

    expect_function_call(SysTime_Init);
    expect_value(Serial_Init, baud_rate, BAUD_RATE);
    expect_function_call(Logging_Init);
    expect_function_call(SystemMonitor_Init);
    will_return(Board_GetNVSAddress, 0x801F800);
    will_return(Board_GetNumberOfPagesInNVS, 2);
    expect_function_call(NVS_Init);
    expect_function_call(CANInterface_Init);
    expect_function_call(ADC_Init);
    expect_function_call(MotorController_Init);
    expect_function_call(SignalHandler_Init);
    will_return(Logging_GetLogger, dummy_logger);
    expect_function_call(CANInterface_RegisterListener);
    expect_any(CANInterface_AddFilter, id);
    expect_any(CANInterface_AddFilter, mask);
    expect_any_always(SignalHandler_RegisterHandler, id);
    will_return_maybe(Board_GetResetFlags, 0);
    will_return_maybe(Board_GetHardwareRevision, 1);
    will_return_maybe(Board_GetSoftwareRevision, 2);
    expect_value(MotorController_SetRPM, index, 0);
    expect_value(MotorController_SetRPM, rpm, 0);
    expect_value(MotorController_SetCurrent, index, 0);
    expect_value(MotorController_SetCurrent, current, 2000);

    Application_Init();
}

static void test_Application_Run(void **state)
{
    const uint32_t motor_status_period_ms = 200;
    const size_t number_of_motors = 1;

    expect_function_call(SignalHandler_Process);
    expect_function_call(MotorController_Update);
    expect_function_call(Console_Process);
    expect_function_call(SystemMonitor_Update);
    will_return(SystemMonitor_GetState, SYSTEM_MONITOR_UNKNOWN);
    will_return(SysTime_GetDifference, motor_status_period_ms - 1);
    Application_Run();

    expect_function_call(SignalHandler_Process);
    expect_function_call(MotorController_Update);
    expect_function_call(Console_Process);
    expect_function_call(SystemMonitor_Update);
    will_return(SystemMonitor_GetState, SYSTEM_MONITOR_UNKNOWN);
    will_return(SysTime_GetDifference, motor_status_period_ms);
    expect_function_call(Board_ToggleStatusLED);

    /* Expect sending motor status */
    const struct motor_controller_motor_status_t status =
    {
        .rpm = {
            .actual = 10,
            .target = 0
        },
        .current = {
            .actual = 150,
            .target = 1000
        },
        .status = MOTOR_COAST
    };
    will_return(Board_GetNumberOfMotors, number_of_motors);
    will_return(MotorController_GetStatus, &status);
    expect_value(SignalHandler_SendMotorStatus, rpm1, status.rpm.actual);
    expect_value(SignalHandler_SendMotorStatus, current1, status.current.actual);
    expect_value(SignalHandler_SendMotorStatus, msg_status_1, status.status);
    expect_value(SignalHandler_SendMotorStatus, rpm2, 0);
    expect_value(SignalHandler_SendMotorStatus, current2, 0);
    expect_value(SignalHandler_SendMotorStatus, msg_status_2, 0);
    will_return(SignalHandler_SendMotorStatus, true);

    will_return(SysTime_GetSystemTime, 0);
    Application_Run();
}

static void test_Application_Run_StateChanges(void **state)
{
    const uint32_t motor_status_period_ms = 200;
    const size_t number_of_motors = 2;
    will_return_always(Board_GetNumberOfMotors, number_of_motors);

    /* Active */
    expect_function_call(SignalHandler_Process);
    expect_function_call(MotorController_Update);
    expect_function_call(Console_Process);
    expect_function_call(SystemMonitor_Update);
    will_return(SystemMonitor_GetState, SYSTEM_MONITOR_ACTIVE);
    for (size_t i = 0; i < number_of_motors; ++i)
    {
        expect_value(MotorController_Run, index, i);
    }
    will_return(SysTime_GetDifference, 0);
    Application_Run();

    /* Inactive */
    expect_function_call(SignalHandler_Process);
    expect_function_call(MotorController_Update);
    expect_function_call(Console_Process);
    expect_function_call(SystemMonitor_Update);
    will_return(SystemMonitor_GetState, SYSTEM_MONITOR_INACTIVE);
    for (size_t i = 0; i < number_of_motors; ++i)
    {
        expect_value(MotorController_Brake, index, i);
    }
    will_return(SysTime_GetDifference, 0);
    Application_Run();

    /* Emergency */
    expect_function_call(SignalHandler_Process);
    expect_function_call(MotorController_Update);
    expect_function_call(Console_Process);
    expect_function_call(SystemMonitor_Update);
    will_return(SystemMonitor_GetState, SYSTEM_MONITOR_EMERGENCY);
    //will_return_always(Board_GetNumberOfMotors, number_of_motors);
    for (size_t i = 0; i < number_of_motors; ++i)
    {
        expect_value(MotorController_Brake, index, i);
    }
    will_return(SysTime_GetDifference, 0);
    Application_Run();

    /* Unknown */
    expect_function_call(SignalHandler_Process);
    expect_function_call(MotorController_Update);
    expect_function_call(Console_Process);
    expect_function_call(SystemMonitor_Update);
    will_return(SystemMonitor_GetState, SYSTEM_MONITOR_UNKNOWN);
    will_return(SysTime_GetDifference, 0);
    Application_Run();
}

static void test_Application_SignalHandlers(void **state)
{
    uint16_t data;
    struct signal_t signal;
    signal.data_p = &data;

    will_return_maybe(Signal_IDToString, "MockSignal");

    data = 1;
    signal.id = SIGNAL_CONTROL_RPM1;
    expect_value(MotorController_SetRPM, index, 0);
    expect_value(MotorController_SetRPM, rpm, data);
    GetCallback(signal.id)(&signal);

    data = 2;
    signal.id = SIGNAL_CONTROL_CURRENT1;
    expect_value(MotorController_SetCurrent, index, 0);
    expect_value(MotorController_SetCurrent, current, data);
    GetCallback(signal.id)(&signal);
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_application[] =
    {
        cmocka_unit_test(test_Application_Init),
        cmocka_unit_test_setup(test_Application_Run, Setup),
        cmocka_unit_test_setup(test_Application_Run_StateChanges, Setup),
        cmocka_unit_test_setup(test_Application_SignalHandlers, Setup)
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_application, NULL, NULL);
}
