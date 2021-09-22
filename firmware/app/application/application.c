
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

#include <string.h>
#include "board.h"
#include "serial.h"
#include "logging.h"
#include "can_interface.h"
#include "adc.h"
#include "systime.h"
#include "console.h"
#include "motor_controller.h"
#include "motor_controller_cmd.h"
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
    uint32_t status_led_time;
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

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void Application_Init(void)
{
    module = (__typeof__(module)) {0};

    SysTime_Init();
    Serial_Init(BAUD_RATE);
    Logging_Init(SysTime_GetSystemTime);
    CANInterface_Init();
    ADC_Init();
    MotorController_Init();

    module.logger = Logging_GetLogger(APPLICATION_LOGGER_NAME);
    Logging_SetLevel(module.logger, APPLICATION_LOGGER_DEBUG_LEVEL);

    struct board_id_t device_id = Board_GetId();
    Logging_Info(module.logger, "Id=%x%x%x HW=%u SW=%u",
                 device_id.offset_0,
                 device_id.offset_4,
                 device_id.offset_8,
                 Board_GetHardwareRevision(),
                 Board_GetSoftwareRevision()
                );
    Logging_Info(module.logger, "Application ready");

    Console_Init(ConsoleWrite, ConsoleRead);
    RegisterConsoleCommands();

    MotorController_SetRPM(0, 0);
    MotorController_SetCurrent(0, 2000);
}

void Application_Run(void)
{
    MotorController_Update();
    Console_Process();

    const uint32_t led_toggle_period_ms = 500;
    if (SysTime_GetDifference(module.status_led_time) >= led_toggle_period_ms)
    {
        Board_ToggleStatusLED();
        module.status_led_time = SysTime_GetSystemTime();
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
}
