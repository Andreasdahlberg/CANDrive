/**
 * @file   test_device_monitoring.c
 * @author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the Device Monitoring module.
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
#include "memfault/components.h"
#include "utility.h"
#include "logging.h"
#include "isotp.h"
#include "device_monitoring.h"
#include "device_monitoring_cmd.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct logging_logger_t *dummy_logger;
static isotp_status_callback_t rx_cb_fp;
static isotp_status_callback_t tx_cb_fp;

//////////////////////////////////////////////////////////////////////////
//MOCKS
//////////////////////////////////////////////////////////////////////////

void ISOTP_Bind(struct isotp_ctx_t *ctx_p, void *rx_buffer_p, size_t rx_buffer_size, void *tx_buffer_p, size_t tx_buffer_size, uint16_t rx_id, uint16_t tx_id, isotp_status_callback_t rx_callback_fp, isotp_status_callback_t tx_callback_fp)
{
    assert_non_null(ctx_p);
    assert_non_null(rx_buffer_p);
    assert_non_null(tx_buffer_p);

    rx_cb_fp = rx_callback_fp;
    tx_cb_fp = tx_callback_fp;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static int Setup(void **state)
{
    will_return_always(Logging_GetLogger, dummy_logger);
    will_return_always(memfault_platform_boot, 0);
    DeviceMonitoring_Init();

    return 0;
}

void MockCallback(void)
{
    function_called();
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

void test_DeviceMonitoring_Init(void **state)
{
    will_return_always(Logging_GetLogger, dummy_logger);
    will_return_always(memfault_platform_boot, 0);
    DeviceMonitoring_Init();
}

void test_DeviceMonitoring_TimerCallback(void **state)
{
    /* Default expectations for 'Transport_Update()'. */
    will_return_always(ISOTP_IsSending, false);
    will_return_always(memfault_packetizer_get_chunk, true);
    will_return_always(ISOTP_Send, true);
    expect_any_always(ISOTP_Send, data_p);
    expect_function_call_any(ISOTP_Proccess);

    /* No callback set. */
    DeviceMonitoring_Update();

    /* Callback set. */
    const uint32_t timer_period_ms = 1000;
    DeviceMonitoring_SetCallback(timer_period_ms, MockCallback);

    will_return(SysTime_GetDifference, timer_period_ms - 1);
    DeviceMonitoring_Update();

    expect_function_call(MockCallback);
    will_return(SysTime_GetDifference, timer_period_ms);
    will_return(SysTime_GetSystemTime, 0);
    DeviceMonitoring_Update();

    /* Callback removed.*/
    DeviceMonitoring_SetCallback(timer_period_ms, NULL);
    DeviceMonitoring_Update();
}

void test_DeviceMonitoring_TransportLayer(void **state)
{
    /* Send successful. */
    will_return(ISOTP_IsSending, false);
    will_return(memfault_packetizer_get_chunk, true);
    will_return(ISOTP_Send, true);
    expect_any(ISOTP_Send, data_p);
    expect_function_call(ISOTP_Proccess);
    DeviceMonitoring_Update();

    tx_cb_fp(ISOTP_STATUS_WAITING);
    tx_cb_fp(ISOTP_STATUS_DONE);
}

void test_DeviceMonitoring_TransportLayer_ErrorHandling(void **state)
{
    /* No data available, do nothing. */
    will_return(ISOTP_IsSending, false);
    will_return(memfault_packetizer_get_chunk, false);
    expect_function_call(ISOTP_Proccess);
    DeviceMonitoring_Update();

    /* Already sending, do nothing. */
    will_return(ISOTP_IsSending, true);
    expect_function_call(ISOTP_Proccess);
    DeviceMonitoring_Update();

    /* Send failed. */
    will_return(ISOTP_IsSending, false);
    will_return(memfault_packetizer_get_chunk, true);
    will_return(ISOTP_Send, false);
    expect_any(ISOTP_Send, data_p);
    expect_function_call(memfault_packetizer_abort);
    expect_function_call(ISOTP_Proccess);
    DeviceMonitoring_Update();

    /* ISOTP errors. */
    const enum isotp_status_t test_data[] =
    {
        ISOTP_STATUS_TIMEOUT,
        ISOTP_STATUS_LOST_FRAME,
        ISOTP_STATUS_OVERFLOW_ABORT,
        UINT32_MAX
    };
    for (size_t i = 0; i < ElementsIn(test_data); ++i)
    {
        will_return(ISOTP_IsSending, false);
        will_return(memfault_packetizer_get_chunk, true);
        will_return(ISOTP_Send, true);
        expect_any(ISOTP_Send, data_p);
        expect_function_call(ISOTP_Proccess);
        DeviceMonitoring_Update();

        /* Fake callback. */
        expect_function_call(memfault_packetizer_abort);
        tx_cb_fp(test_data[i]);
    }
}

void test_DeviceMonitoring_ResetImminent(void **state)
{
    expect_value(memfault_reboot_tracking_mark_reset_imminent, reboot_reason, kMfltRebootReason_UserReset);
    DeviceMonitoring_ResetImminent(DEV_MON_REBOOT_REAS_USER_RESET);

    expect_value(memfault_reboot_tracking_mark_reset_imminent, reboot_reason, kMfltRebootReason_FirmwareUpdate);
    DeviceMonitoring_ResetImminent(DEV_MON_REBOOT_REAS_FW_UPDATE);

    expect_value(memfault_reboot_tracking_mark_reset_imminent, reboot_reason, kMfltRebootReason_SoftwareReset);
    DeviceMonitoring_ResetImminent(DEV_MON_REBOOT_REAS_SW_RESET);

    expect_value(memfault_reboot_tracking_mark_reset_imminent, reboot_reason, kMfltRebootReason_Unknown);
    DeviceMonitoring_ResetImminent(UINT32_MAX);
}

void test_DeviceMonitoring_Count_InvalidId(void **state)
{
    expect_assert_failure(DeviceMonitoring_Count(UINT32_MAX, 0));
}

void test_DeviceMonitoring_Count(void **state)
{
    will_return_always(memfault_metrics_heartbeat_add, 0);

    const enum device_monitoring_metric_id counter_ids[] =
    {
        DEV_MON_METRIC_CAN_TX_ERROR,
        DEV_MON_METRIC_EMERGENCY_STOP
    };
    for (size_t n = 0; n < ElementsIn(counter_ids); ++n)
    {
        const int32_t test_data[] = {INT32_MIN, 0, INT32_MAX};
        for (size_t i = 0; i < ElementsIn(test_data); ++i)
        {
            expect_value(memfault_metrics_heartbeat_add, amount, test_data[i]);
            DeviceMonitoring_Count(counter_ids[n], test_data[i]);
        }
    }
}

void test_DeviceMonitoring_Timer(void **state)
{
    will_return(memfault_metrics_heartbeat_timer_start, 0);
    DeviceMonitoring_StartTimer(DEV_MON_METRIC_MAIN_TASK_TIME);

    will_return(memfault_metrics_heartbeat_timer_stop, 0);
    DeviceMonitoring_StopTimer(DEV_MON_METRIC_MAIN_TASK_TIME);
}

void test_DeviceMonitoringCmd_DumpData(void **state)
{
    expect_function_call(memfault_data_export_dump_chunks);
    assert_true(DeviceMonitoringCmd_DumpData());
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_device_monitoring[] =
    {
        cmocka_unit_test(test_DeviceMonitoring_Init),
        cmocka_unit_test_setup(test_DeviceMonitoring_TimerCallback, Setup),
        cmocka_unit_test_setup(test_DeviceMonitoring_TransportLayer, Setup),
        cmocka_unit_test_setup(test_DeviceMonitoring_TransportLayer_ErrorHandling, Setup),
        cmocka_unit_test_setup(test_DeviceMonitoring_ResetImminent, Setup),
        cmocka_unit_test_setup(test_DeviceMonitoring_Count_InvalidId, Setup),
        cmocka_unit_test_setup(test_DeviceMonitoring_Count, Setup),
        cmocka_unit_test_setup(test_DeviceMonitoring_Timer, Setup)
    };

    const struct CMUnitTest test_device_monitoring_cmd[] =
    {
        cmocka_unit_test(test_DeviceMonitoringCmd_DumpData)
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    int status = 0;
    status = cmocka_run_group_tests(test_device_monitoring, NULL, NULL);
    status += cmocka_run_group_tests(test_device_monitoring_cmd, NULL, NULL);
    return status;
}
