/**
 * @file   device_monitoring.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
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

#ifndef DEVICE_MONITORING_H_
#define DEVICE_MONITORING_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

enum device_monitoring_reboot_reason
{
    DEV_MON_REBOOT_REAS_USER_RESET = 1,
    DEV_MON_REBOOT_REAS_FW_UPDATE,
    DEV_MON_REBOOT_REAS_SW_RESET,
};

enum device_monitoring_metric_id
{
    DEV_MON_METRIC_CAN_TX_ERROR = 1,
    DEV_MON_METRIC_EMERGENCY_STOP,
    DEV_MON_METRIC_MAIN_TASK_TIME,
};

typedef void (*device_monitoring_timer_cb_t)(void);

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize the device monitoring module.
 */
void DeviceMonitoring_Init(void);

/**
 * Update the internal state of the device monitoring module.
 */
void DeviceMonitoring_Update(void);

/**
 * Set a timed callback.
 *
 * @param period_sec Period between calls.
 * @param timer_callback Callback.
 */
void DeviceMonitoring_SetCallback(uint32_t period_sec, device_monitoring_timer_cb_t timer_callback);

/**
 * Indicate that a reset is imminent and store the reason.
 *
 * @param reason Imminent reset reason.
 */
void DeviceMonitoring_ResetImminent(enum device_monitoring_reboot_reason reason);

/**
 * Add an arbitrary value to the specified metric.
 *
 * @param id Metric ID.
 * @param amount Amount to add to metric.
 */
void DeviceMonitoring_Count(enum device_monitoring_metric_id id, int32_t amount);

/**
 * Start a timer.
 * @param id ID of timer.
 */
void DeviceMonitoring_StartTimer(enum device_monitoring_metric_id id);

/**
 * Stop a timer.
 * @param id ID of timer.
 */
void DeviceMonitoring_StopTimer(enum device_monitoring_metric_id id);

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

#endif
