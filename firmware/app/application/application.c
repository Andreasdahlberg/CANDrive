
/**
 * @file   application.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  CANDrive application.
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
#include <string.h>
#include "board.h"
#include "board_cmd.h"
#include "serial.h"
#include "logging.h"
#include "logging_cmd.h"
#include "can_interface.h"
#include "adc.h"
#include "systime.h"
#include "console.h"
#include "motor_controller.h"
#include "motor_controller_cmd.h"
#include "signal_handler.h"
#include "system_monitor.h"
#include "utility.h"
#include "application.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define APPLICATION_LOGGER_NAME "App"
#ifndef APPLICATION_LOGGER_DEBUG_LEVEL
#define APPLICATION_LOGGER_DEBUG_LEVEL LOGGING_DEBUG
#endif

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct module_t
{
    logging_logger_t *logger;
    uint32_t motor_status_time;
    enum system_monitor_state_t last_state;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct module_t module;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static void ConsoleWrite(const char *str);
static size_t ConsoleRead(char *str);
static void RegisterConsoleCommands(void);
static void ConfigureSignalHandler(void);
static void HandleRPM1Signal(struct signal_t *signal_p);
static void HandleCurrent1Signal(struct signal_t *signal_p);
static void HandleStateChanges(void);
static void HandleActiveState(void);
static void HandleInactiveState(void);
static inline void PrintResetFlags(void);
static inline void PrintIdAndRevision(void);
static void SendMotorStatus(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void Application_Init(void)
{
    module = (__typeof__(module)) {0};

    SysTime_Init();
    Serial_Init(BAUD_RATE);
    Logging_Init(SysTime_GetSystemTime);

    SystemMonitor_Init();
    CANInterface_Init();
    ADC_Init();
    MotorController_Init();
    SignalHandler_Init();

    module.logger = Logging_GetLogger(APPLICATION_LOGGER_NAME);
    Logging_SetLevel(module.logger, APPLICATION_LOGGER_DEBUG_LEVEL);

    ConfigureSignalHandler();

    PrintResetFlags();
    PrintIdAndRevision();

    Logging_Info(module.logger, "Application ready");

    Console_Init(ConsoleWrite, ConsoleRead);
    RegisterConsoleCommands();

    MotorController_SetRPM(0, 0);
    MotorController_SetCurrent(0, 2000);
}

void Application_Run(void)
{
    SignalHandler_Process();
    MotorController_Update();
    Console_Process();
    SystemMonitor_Update();
    HandleStateChanges();

    const uint32_t motor_status_period_ms = 200;
    if (SysTime_GetDifference(module.motor_status_time) >= motor_status_period_ms)
    {
        Board_ToggleStatusLED();
        SendMotorStatus();
        module.motor_status_time = SysTime_GetSystemTime();
    }
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static void ConsoleWrite(const char *str)
{
    Serial_Send(str, strlen(str));
}

static size_t ConsoleRead(char *str)
{
    return Serial_Read(str, 1);
}

static void RegisterConsoleCommands(void)
{
    Console_RegisterCommand("rpm", MotorControllerCmd_SetRPM);
    Console_RegisterCommand("current", MotorControllerCmd_SetCurrent);
    Console_RegisterCommand("run", MotorControllerCmd_Run);
    Console_RegisterCommand("coast", MotorControllerCmd_Coast);
    Console_RegisterCommand("brake", MotorControllerCmd_Brake);
    Console_RegisterCommand("reset", BoardCmd_Reset);
    Console_RegisterCommand("level", LoggingCmd_SetLevel);
}

static void ConfigureSignalHandler(void)
{
    const uint32_t motor_control_frame_id = 0x09;
    const uint32_t id_mask = 0xffff;

    CANInterface_RegisterListener(SignalHandler_Listener);
    CANInterface_AddFilter(motor_control_frame_id, id_mask);
    SignalHandler_RegisterHandler(SIGNAL_CONTROL_RPM1, HandleRPM1Signal);
    SignalHandler_RegisterHandler(SIGNAL_CONTROL_CURRENT1, HandleCurrent1Signal);
}

static void HandleRPM1Signal(struct signal_t *signal_p)
{
    Signal_Log(signal_p, module.logger);
    MotorController_SetRPM(BOARD_M1_INDEX, *(int16_t *)signal_p->data_p);
}

static void HandleCurrent1Signal(struct signal_t *signal_p)
{
    Signal_Log(signal_p, module.logger);
    MotorController_SetCurrent(BOARD_M1_INDEX, *(int16_t *)signal_p->data_p);
}

static void HandleStateChanges(void)
{
    const enum system_monitor_state_t state = SystemMonitor_GetState();

    if (module.last_state != state)
    {
        module.last_state = state;

        switch (state)
        {
            case SYSTEM_MONITOR_ACTIVE:
                HandleActiveState();
                break;

            case SYSTEM_MONITOR_INACTIVE:
                HandleInactiveState();
                break;

            default:
                break;

        }
    }
}

static void HandleActiveState(void)
{
    const size_t number_of_motors = Board_GetNumberOfMotors();
    for (size_t i = 0; i < number_of_motors; ++i)
    {
        MotorController_Run(i);
    }
}

static void HandleInactiveState(void)
{
    const size_t number_of_motors = Board_GetNumberOfMotors();
    for (size_t i = 0; i < number_of_motors; ++i)
    {
        MotorController_Brake(i);
    }
}

static inline void PrintResetFlags(void)
{
    const uint32_t reset_flags = Board_GetResetFlags();
    Logging_Info(module.logger, "reset_flags: {LPWRR: %u, IWDGRSTF: %u, PINRSTF: %u, SFTRSTF: %u}",
                 !!(reset_flags & RCC_CSR_LPWRRSTF),
                 !!(reset_flags & RCC_CSR_IWDGRSTF),
                 !!(reset_flags & RCC_CSR_PINRSTF),
                 !!(reset_flags & RCC_CSR_SFTRSTF)
                );
}

static inline void PrintIdAndRevision(void)
{
    struct board_id_t device_id = Board_GetId();
    Logging_Info(module.logger, "Id=%x%x%x HW=%u SW=%u",
                 device_id.offset_0,
                 device_id.offset_4,
                 device_id.offset_8,
                 Board_GetHardwareRevision(),
                 Board_GetSoftwareRevision()
                );
}

static void SendMotorStatus(void)
{
    struct motor_controller_motor_status_t motors[2] = {0};

    const size_t number_of_motors = Board_GetNumberOfMotors();
    assert(number_of_motors <= ElementsIn(motors));
    for (size_t i = 0; i < number_of_motors; ++i)
    {
        motors[i] = MotorController_GetStatus(i);
    }

    SignalHandler_SendMotorStatus(motors[0].rpm.actual,
                                  motors[0].current.actual,
                                  motors[1].rpm.actual,
                                  motors[1].current.actual,
                                  0);
}
