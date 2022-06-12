/**
 * @file   system_monitor.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  System monitor module.
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
#include <stddef.h>
#include <libopencm3/stm32/iwdg.h>
#include "utility.h"
#include "logging.h"
#include "board.h"
#include "systime.h"
#include "nvcom.h"
#include "system_monitor.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define SYSTEMMONITOR_LOGGER_NAME "SysMon"
#ifndef SYSTEMMONITOR_LOGGER_DEBUG_LEVEL
#define SYSTEMMONITOR_LOGGER_DEBUG_LEVEL LOGGING_INFO
#endif

#define WATCHDOG_PERIOD_MS 200
#define MAX_NUMBER_OF_WATCHDOG_HANDLES 32
#define MAGIC_NUMBER 0xABCD
#define CONTROL_INACTIVITY_PERIOD_MS 200

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct module_t
{
    logging_logger_t *logger;
    uint32_t number_of_handles;
    uint32_t flags;
    uint32_t control_activity_timer;
    enum system_monitor_state_t state;
    struct nvcom_data_t *restart_information_p;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct module_t module;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static inline bool IsColdRestart(void);
static inline uint32_t GetRequiredFlags(void);
static inline bool IsWatchdogRestart(void);
static void UpdateRestartInformation(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void SystemMonitor_Init(void)
{
    module = (__typeof__(module)) {0};
    module.state = SYSTEM_MONITOR_INACTIVE;
    module.restart_information_p = NVCom_GetData();

    module.logger = Logging_GetLogger("SysMon");
    Logging_SetLevel(module.logger, SYSTEMMONITOR_LOGGER_DEBUG_LEVEL);

    UpdateRestartInformation();

    Logging_Info(module.logger, "{restarts: %u, wdt_restarts: %u, cold: %u, wdt: %u}",
                 module.restart_information_p->number_of_restarts,
                 module.restart_information_p->number_of_watchdog_restarts,
                 (uint32_t)IsColdRestart(),
                 (uint32_t)IsWatchdogRestart()
                );

    assert(module.restart_information_p->number_of_watchdog_restarts < 3 && "Stopped due to watchdog reset loop");
    iwdg_set_period_ms(WATCHDOG_PERIOD_MS);
    iwdg_start();

    Logging_Info(module.logger, "SystemMonitor initialized {state: SYSTEM_MONITOR_INACTIVE}");
}

void SystemMonitor_Update(void)
{
    assert(module.number_of_handles > 0);

    if (GetRequiredFlags() == module.flags)
    {
        iwdg_reset();
        module.flags = 0;
    }

    if (Board_GetEmergencyPinState())
    {
        if (module.state != SYSTEM_MONITOR_EMERGENCY)
        {
            module.state = SYSTEM_MONITOR_EMERGENCY;
            Logging_Info(module.logger, "{state: SYSTEM_MONITOR_EMERGENCY}");
        }
    }
    else
    {

        if ((module.state != SYSTEM_MONITOR_INACTIVE) &&
                (SysTime_GetDifference(module.control_activity_timer) > CONTROL_INACTIVITY_PERIOD_MS))
        {
            Logging_Info(module.logger, "{state: SYSTEM_MONITOR_INACTIVE}");
            module.state = SYSTEM_MONITOR_INACTIVE;
        }
    }
}

uint32_t SystemMonitor_GetWatchdogHandle(void)
{
    assert(module.number_of_handles < MAX_NUMBER_OF_WATCHDOG_HANDLES);

    const uint32_t handle = module.number_of_handles;
    ++module.number_of_handles;

    SystemMonitor_FeedWatchdog(handle);
    return handle;
}

void SystemMonitor_FeedWatchdog(uint32_t handle)
{
    assert(handle < module.number_of_handles);

    module.flags |= (1 << handle);
}

void SystemMonitor_ReportActivity(void)
{
    if (!Board_GetEmergencyPinState())
    {
        module.state = SYSTEM_MONITOR_ACTIVE;
    }
    module.control_activity_timer = SysTime_GetSystemTime();
}

enum system_monitor_state_t SystemMonitor_GetState(void)
{
    return module.state;
}

uint32_t SystemMonitor_GetResetFlags(void)
{
    const struct nvcom_data_t *data_p = NVCom_GetData();
    return data_p->reset_flags;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static inline bool IsColdRestart(void)
{
    return module.restart_information_p->number_of_restarts == 0;
}

static inline uint32_t GetRequiredFlags(void)
{
    uint32_t required_flags = 0;
    for (size_t i = 0; i < module.number_of_handles; ++i)
    {
        required_flags |= (1 << i);
    }

    return required_flags;
}

static inline bool IsWatchdogRestart(void)
{
    uint32_t reset_flags = SystemMonitor_GetResetFlags();

    return (bool)(reset_flags & RCC_CSR_IWDGRSTF);
}

static void UpdateRestartInformation(void)
{
    if (IsWatchdogRestart())
    {
        ++module.restart_information_p->number_of_watchdog_restarts;
        Logging_Error(module.logger, "Restarted due to watchdog timeout!");
    }
    else
    {
        module.restart_information_p->number_of_watchdog_restarts = 0;
    }

    module.restart_information_p->request_firmware_update = false;
    NVCom_SetData(module.restart_information_p);
}
