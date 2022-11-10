/**
 * @file   memfault_platform_port.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Platform port layer.
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
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "memfault/components.h"
#include "memfault/ports/reboot_reason.h"

#include "board.h"
#include "system_monitor.h"
#include "systime.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#ifndef SOFTWARE_VERSION
#define SOFTWARE_VERSION "0.0.0"
#endif

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

MEMFAULT_PUT_IN_SECTION(".noinit.mflt_reboot_tracking")
static uint8_t s_reboot_tracking[MEMFAULT_REBOOT_TRACKING_REGION_SIZE];

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static char *get_device_id(void);
static char *get_hardware_revision(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void memfault_platform_get_device_info(sMemfaultDeviceInfo *info)
{
    *info = (sMemfaultDeviceInfo)
    {
        .device_serial = get_device_id(),
         .software_type = "appfw",
          .software_version = SOFTWARE_VERSION,
           .hardware_version = get_hardware_revision(),
    };
}

void memfault_platform_reboot(void)
{
    Board_Reset();
    while (1)
    {
        /* Should never come here. */
    }
}

bool memfault_platform_time_get_current(sMemfaultCurrentTime *time __attribute__((unused)))
{
    return false;
}


int memfault_platform_boot(void)
{
    memfault_platform_reboot_tracking_boot();

    // initialize the event storage buffer
    static uint8_t s_event_storage[1024];
    const sMemfaultEventStorageImpl *evt_storage =
        memfault_events_storage_boot(s_event_storage, sizeof(s_event_storage));

    // configure trace events to store into the buffer
    memfault_trace_event_boot(evt_storage);

    // record the current reboot reason
    memfault_reboot_tracking_collect_reset_info(evt_storage);

    // configure the metrics component to store into the buffer
    sMemfaultMetricBootInfo boot_info =
    {
        .unexpected_reboot_count = memfault_reboot_tracking_get_crash_count(),
    };
    memfault_metrics_boot(evt_storage, &boot_info);

    return 0;
}

void memfault_reboot_reason_get(sResetBootupInfo *info)
{
    const uint32_t reset_cause = SystemMonitor_GetResetFlags();

    eMemfaultRebootReason reset_reason;

    if (reset_cause & RCC_CSR_SFTRSTF)
    {
        reset_reason = kMfltRebootReason_SoftwareReset;
    }
    else if (reset_cause & RCC_CSR_IWDGRSTF)
    {
        reset_reason = kMfltRebootReason_HardwareWatchdog;
    }
    else if (reset_cause & RCC_CSR_PORRSTF)
    {
        reset_reason = kMfltRebootReason_PowerOnReset;
    }
    else if (reset_cause & RCC_CSR_PINRSTF)
    {
        reset_reason = kMfltRebootReason_PinReset;
    }
    else
    {
        reset_reason = kMfltRebootReason_Unknown;
    }

    *info = (sResetBootupInfo)
    {
        .reset_reason_reg = reset_cause,
         .reset_reason = reset_reason,
    };
}

void memfault_platform_log(eMemfaultPlatformLogLevel level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char log_buf[128];
    vsnprintf(log_buf, sizeof(log_buf), fmt, args);

    const char *lvl_str = "";
    switch (level)
    {
        case kMemfaultPlatformLogLevel_Debug:
            lvl_str = "D";
            break;

        case kMemfaultPlatformLogLevel_Info:
            lvl_str = "I";
            break;

        case kMemfaultPlatformLogLevel_Warning:
            lvl_str = "W";
            break;

        case kMemfaultPlatformLogLevel_Error:
            lvl_str = "E";
            break;

        default:
            break;
    }

    vsnprintf(log_buf, sizeof(log_buf), fmt, args);

    printf("[%s] MFLT: %s\n", lvl_str, log_buf);
}

void memfault_platform_reboot_tracking_boot(void)
{
    sResetBootupInfo reset_info = { 0 };
    memfault_reboot_reason_get(&reset_info);
    memfault_reboot_tracking_boot(s_reboot_tracking, &reset_info);
}

bool memfault_platform_metrics_timer_boot(uint32_t period_sec __attribute__((unused)),
        MemfaultPlatformTimerCallback callback __attribute__((unused)))
{
    return false;
}

uint64_t memfault_platform_get_time_since_boot_ms(void)
{
    return SysTime_GetSystemTime();
}

void __assert_func(const char *file __attribute__((unused)),
                   int line __attribute__((unused)),
                   const char *func __attribute__((unused)),
                   const char *failedexpr __attribute__((unused)))
{
    MEMFAULT_ASSERT(0);
}

void _exit(int status __attribute__((unused)))
{
    MEMFAULT_ASSERT(0);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static char *get_device_id(void)
{
    struct board_id_t id = Board_GetId();
    static char device_id[(96 / 8) * 2 + 1];

    snprintf(device_id, sizeof(device_id), "%lx%lx%lx",
             id.offset_0,
             id.offset_4,
             id.offset_8
            );

    return device_id;
}

static char *get_hardware_revision(void)
{
    static char hardware_revision[6];
    snprintf(hardware_revision, sizeof(hardware_revision), "rev%lu", Board_GetHardwareRevision());
    return hardware_revision;
}
