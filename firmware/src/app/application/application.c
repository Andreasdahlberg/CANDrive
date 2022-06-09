
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
#include <stdio.h>
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
#include "nvs.h"
#include "nvs_cmd.h"
#include "config.h"
#include "flash.h"
#include "image.h"
#include "nvcom.h"
#include "application.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define APPLICATION_LOGGER_NAME "App"
#ifndef APPLICATION_LOGGER_DEBUG_LEVEL
#define APPLICATION_LOGGER_DEBUG_LEVEL LOGGING_INFO
#endif

#ifndef GIT_DESC
#define GIT_DESC "UNKNOWN"
#endif

#ifndef SOFTWARE_VERSION
#define SOFTWARE_VERSION "UNKNOWN"
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

struct note_section_t
{
    uint32_t namesz;
    uint32_t descsz;
    uint32_t type;
    uint8_t data[];
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

struct image_header_t header __attribute__((section(".image_header"))) =
{
    .header_magic = IMAGE_HEADER_MAGIC,
    .header_version = 0,
    .image_type = IMAGE_TYPE_CANDRIVE_APP,
    .version = SOFTWARE_VERSION,
    .vector_address = (uintptr_t) &vector_table,
    .git_sha = GIT_DESC,
    /* Populated after compilation */
    .crc = 0,
    .size = 0,
};
extern const struct note_section_t note_build_id;
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
static void HandleMode1Signal(struct signal_t *signal_p);
static void HandleRPM2Signal(struct signal_t *signal_p);
static void HandleCurrent2Signal(struct signal_t *signal_p);
static void HandleMode2Signal(struct signal_t *signal_p);
static void HandleModeSignal(struct signal_t *signal_p, uint8_t index);
static void HandleStateChanges(void);
static void BrakeAllMotors(void);
static inline void PrintResetFlags(void);
static inline void PrintHardwareInformation(void);
static inline void PrintSoftwareInformation(void);
static inline void PrintConfig(void);
static void SendMotorStatus(void);
static size_t GetBuildID(char *build_id, size_t length);
static uint32_t GetResetFlags(void);
static bool EnterUpdateMode(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void Application_Init(void)
{
    module = (__typeof__(module)) {0};

    SysTime_Init();
    Serial_Init(BAUD_RATE);
    Logging_Init(SysTime_GetSystemTime);

    NVCom_Init();
    SystemMonitor_Init();
    Flash_Init();
    NVS_Init(Board_GetNVSAddress(), Board_GetNumberOfPagesInNVS());
    Config_Init();
    CANInterface_Init();
    ADC_Init();
    MotorController_Init();
    SignalHandler_Init();

    module.logger = Logging_GetLogger(APPLICATION_LOGGER_NAME);
    Logging_SetLevel(module.logger, APPLICATION_LOGGER_DEBUG_LEVEL);

    ConfigureSignalHandler();

    PrintResetFlags();
    PrintHardwareInformation();
    PrintSoftwareInformation();
    PrintConfig();

    Logging_Info(module.logger, "%s ready", NAME);

    Console_Init(ConsoleWrite, ConsoleRead);
    RegisterConsoleCommands();
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
    Console_RegisterCommand("store", NVSCmd_Store);
    Console_RegisterCommand("remove", NVSCmd_Remove);
    Console_RegisterCommand("update", EnterUpdateMode);
}

static void ConfigureSignalHandler(void)
{
    const uint32_t motor_control_frame_id = 0x09;
    const uint32_t id_mask = 0xffff;

    CANInterface_RegisterListener(SignalHandler_Listener, NULL);
    CANInterface_AddFilter(motor_control_frame_id, id_mask);
    if (Config_GetNumberOfMotors() > 0)
    {
        SignalHandler_RegisterHandler(SIGNAL_CONTROL_RPM1, HandleRPM1Signal);
        SignalHandler_RegisterHandler(SIGNAL_CONTROL_CURRENT1, HandleCurrent1Signal);
        SignalHandler_RegisterHandler(SIGNAL_CONTROL_MODE1, HandleMode1Signal);
    }
    if (Config_GetNumberOfMotors() > 1)
    {
        SignalHandler_RegisterHandler(SIGNAL_CONTROL_RPM2, HandleRPM2Signal);
        SignalHandler_RegisterHandler(SIGNAL_CONTROL_CURRENT2, HandleCurrent2Signal);
        SignalHandler_RegisterHandler(SIGNAL_CONTROL_MODE2, HandleMode2Signal);
    }
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

static void HandleMode1Signal(struct signal_t *signal_p)
{
    HandleModeSignal(signal_p, BOARD_M1_INDEX);
}

static void HandleRPM2Signal(struct signal_t *signal_p)
{
    Signal_Log(signal_p, module.logger);
    MotorController_SetRPM(BOARD_M2_INDEX, *(int16_t *)signal_p->data_p);
}

static void HandleCurrent2Signal(struct signal_t *signal_p)
{
    Signal_Log(signal_p, module.logger);
    MotorController_SetCurrent(BOARD_M2_INDEX, *(int16_t *)signal_p->data_p);
}

static void HandleMode2Signal(struct signal_t *signal_p)
{
    HandleModeSignal(signal_p, BOARD_M2_INDEX);
}

static void HandleModeSignal(struct signal_t *signal_p, uint8_t index)
{
    Signal_Log(signal_p, module.logger);

    if (SystemMonitor_GetState() != SYSTEM_MONITOR_EMERGENCY)
    {
        const uint8_t mode = *(uint8_t *)signal_p->data_p;
        switch (mode)
        {
            case 0:
                /* Do nothing */
                break;
            case 1:
                MotorController_Run(index);
                break;
            case 2:
                MotorController_Coast(index);
                break;
            case 3:
                MotorController_Brake(index);
                break;
            default:
                Logging_Warning(module.logger, "Unknown mode: {index: %u, mode: %u}", index, mode);
        }
    }
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
                /* Do nothing */
                break;

            case SYSTEM_MONITOR_EMERGENCY:
            case SYSTEM_MONITOR_INACTIVE:
                BrakeAllMotors();
                break;

            default:
                break;
        }
    }
}

static void BrakeAllMotors(void)
{
    const size_t number_of_motors = Config_GetNumberOfMotors();
    for (size_t i = 0; i < number_of_motors; ++i)
    {
        MotorController_Brake(i);
    }
}

static inline void PrintResetFlags(void)
{
    const uint32_t reset_flags = GetResetFlags();
    Logging_Info(module.logger, "reset_flags: {LPWRR: %u, IWDGRSTF: %u, PINRSTF: %u, SFTRSTF: %u}",
                 !!(reset_flags & RCC_CSR_LPWRRSTF),
                 !!(reset_flags & RCC_CSR_IWDGRSTF),
                 !!(reset_flags & RCC_CSR_PINRSTF),
                 !!(reset_flags & RCC_CSR_SFTRSTF)
                );
}

static inline void PrintHardwareInformation(void)
{
    struct board_id_t device_id = Board_GetId();
    Logging_Info(module.logger, "hw_info: {id: %x%x%x, rev: %u}",
                 device_id.offset_0,
                 device_id.offset_4,
                 device_id.offset_8,
                 Board_GetHardwareRevision()
                );
}

static inline void PrintSoftwareInformation(void)
{
    char build_id[41];
    GetBuildID(build_id, sizeof(build_id));
    Logging_Info(module.logger, "sw_info: {sw: %s, sha: %s, build_id: %s}",
                 SOFTWARE_VERSION,
                 GIT_DESC,
                 build_id
                );
}

static inline void PrintConfig(void)
{
    Logging_Info(module.logger, "config: {valid: %s, number_of_motors: %u, counts_per_rev: %u, no_load_rpm: %u, no_load_current: %u, stall_current: %u, kp: %u, ki: %u, kd: %u, imax: %i, imin: %i}",
                 Config_IsValid() ? "true" : "false",
                 Config_GetNumberOfMotors(),
                 Config_GetCountsPerRev(),
                 Config_GetNoLoadRpm(),
                 Config_GetNoLoadCurrent(),
                 Config_GetStallCurrent(),
                 Config_GetValue("kp"),
                 Config_GetValue("ki"),
                 Config_GetValue("kd"),
                 Config_GetValue("imax"),
                 Config_GetValue("imin")
                );
}

static void SendMotorStatus(void)
{
    struct motor_controller_motor_status_t motors[2] = {0};

    const size_t number_of_motors = Config_GetNumberOfMotors();
    assert(number_of_motors <= ElementsIn(motors));
    for (size_t i = 0; i < number_of_motors; ++i)
    {
        motors[i] = MotorController_GetStatus(i);
    }

    SignalHandler_SendMotorStatus(motors[0].rpm.actual,
                                  motors[0].current.actual,
                                  (uint8_t)motors[0].status,
                                  motors[1].rpm.actual,
                                  motors[1].current.actual,
                                  (uint8_t)motors[1].status);
}

static size_t GetBuildID(char *build_id, size_t length)
{
    assert(length > 0);

    size_t number_of_bytes = note_build_id.descsz > length / 2 ? length / 2 : note_build_id.descsz;
    const uint8_t *build_id_data = &note_build_id.data[note_build_id.namesz];

    size_t number_of_chars = 0;
    for (size_t i = 0; i < number_of_bytes; ++i)
    {
        number_of_chars += snprintf(&build_id[number_of_chars], length, "%02x", build_id_data[i]);
    }

    return number_of_chars;
}

static uint32_t GetResetFlags(void)
{
    struct nvcom_data_t *data_p = NVCom_GetData();
    return data_p->reset_flags;
}

static bool EnterUpdateMode(void)
{
    struct nvcom_data_t *data_p = NVCom_GetData();
    data_p->request_firmware_update = true;
    NVCom_SetData(data_p);
    Board_Reset();

    return true;
}
