/**
 * @file   device_monitoring.c
 * @author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Device monitoring module.
 */

/*
This file is part of SillyCat firmware.

SillyCat firmware is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SillyCat firmware is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SillyCat firmware.  If not, see <http://www.gnu.org/licenses/>.
*/

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stddef.h>
#include "memfault/components.h"
#include "logging.h"
#include "device_monitoring.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define DEVICE_MONITORING_LOGGER_NAME "DevMo"
#ifndef DEVICE_MONITORING_LOGGER_DEBUG_LEVEL
#define DEVICE_MONITORING_LOGGER_DEBUG_LEVEL LOGGING_INFO
#endif

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct module_t
{
    logging_logger_t *logger_p;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct module_t module;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

eMemfaultRebootReason ResetReasonToMemfault(enum device_monitoring_reboot_reason reason);
MemfaultMetricId MetricIdToMemfault(enum device_monitoring_metric_id id);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void DeviceMonitoring_Init(void)
{
    module = (__typeof__(module)) {0};
    module.logger_p = Logging_GetLogger(DEVICE_MONITORING_LOGGER_NAME);
    Logging_SetLevel(module.logger_p, DEVICE_MONITORING_LOGGER_DEBUG_LEVEL);

    memfault_platform_boot();

    Logging_Info(module.logger_p, "DeviceMonitoring initialized {type: mflt}");
}

void DeviceMonitoring_ResetImminent(enum device_monitoring_reboot_reason reason)
{
    Logging_Info(module.logger_p, "Reset imminent {reason: %d}", reason);

    memfault_reboot_tracking_mark_reset_imminent(ResetReasonToMemfault(reason), NULL);
}

void DeviceMonitoring_Count(enum device_monitoring_metric_id id, int32_t amount)
{
    memfault_metrics_heartbeat_add(MetricIdToMemfault(id), amount);
}

void DeviceMonitoring_StartTimer(enum device_monitoring_metric_id id)
{
    memfault_metrics_heartbeat_timer_start(MetricIdToMemfault(id));
}

void DeviceMonitoring_StopTimer(enum device_monitoring_metric_id id)
{
    memfault_metrics_heartbeat_timer_stop(MetricIdToMemfault(id));
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

eMemfaultRebootReason ResetReasonToMemfault(enum device_monitoring_reboot_reason reason)
{
    switch (reason)
    {
        case DEVICE_MONITORING_REBOOT_REASON_USER_RESET:
            return kMfltRebootReason_UserReset;

        case DEVICE_MONITORING_REBOOT_REASON_FIRMWARE_UPDATE:
            return kMfltRebootReason_FirmwareUpdate;

        case DEVICE_MONITORING_REBOOT_REASON_SOFTWARE_RESET:
            return kMfltRebootReason_SoftwareReset;

        default:
            return kMfltRebootReason_Unknown;
    }
}

MemfaultMetricId MetricIdToMemfault(enum device_monitoring_metric_id id)
{

    switch (id)
    {
        case DEVICE_MONITORING_METRIC_CAN_TX_ERROR:
            return MEMFAULT_METRICS_KEY(can_tx_error);

        case DEVICE_MONITORING_METRIC_EMERGENCY_STOP:
            return MEMFAULT_METRICS_KEY(emergency_stop);
    }

    assert(0);
}
