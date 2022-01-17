/**
 * @file   test_isotp.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the ISO15765-2/ISO-TP module.
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
#include "can_interface.h"
#include "isotp.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct listener_t
{
    caninterface_listener_cb_t listener_cb;
    void *arg_p;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct isotp_ctx_t ctx;
static struct logging_logger_t *dummy_logger;
static struct listener_t rx_listener;
static struct listener_t tx_listener;
static bool drop_frame;
static bool got_callback;

//////////////////////////////////////////////////////////////////////////
//MOCKS
//////////////////////////////////////////////////////////////////////////

void CANInterface_RegisterListener(caninterface_listener_cb_t listener_cb, void *arg_p)
{
    if (rx_listener.listener_cb == NULL)
    {
        rx_listener.listener_cb = listener_cb;
        rx_listener.arg_p = arg_p;
    }
    else
    {
        tx_listener.listener_cb = listener_cb;
        tx_listener.arg_p = arg_p;
    }
}

bool CANInterface_Transmit(uint32_t id, void *data_p, size_t size)
{
    struct can_frame_t frame;
    frame.id = id;
    frame.size = size;
    memcpy(frame.data, data_p, size);

    if (!drop_frame)
    {
        /**
         * Create a loop back by changing the TX ID and directing frames by
         * ISOTP frame type to the correct listener.
         */
        frame.id = 0x01;
        if ((frame.data[0] & 0xf0) < 0x30)
        {
            rx_listener.listener_cb(&frame, rx_listener.arg_p);
        }
        else
        {
            tx_listener.listener_cb(&frame, tx_listener.arg_p);
        }
    }
    drop_frame = false;
    return mock_type(bool);
}

static void InjectFlowControlFrame(uint32_t id)
{
    struct can_frame_t frame;
    frame.id = id;
    frame.size = 3;
    frame.data[0] = 0x31;

    CANInterface_Transmit(frame.id, frame.data,frame.size);
}

static void MockRxStatusHandler(enum isotp_status_t status)
{
    check_expected(status);
    got_callback = true;
}

static void MockTxStatusHandler(enum isotp_status_t status)
{
    check_expected(status);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static int Setup(void **state)
{
    ctx = (__typeof__(ctx)) {0};
    rx_listener = (__typeof__(rx_listener)) {0};
    tx_listener = (__typeof__(tx_listener)) {0};
    drop_frame = false;
    got_callback = false;
    return 0;
}

static void ProccessUntilStatus(struct isotp_ctx_t *ctx_p, enum isotp_status_t expected_status)
{
    expect_value(MockRxStatusHandler, status, expected_status);

    /* Max iterations is used as a timeout if the status callback is never called. */
    const size_t max_iterations = 2000;
    for (size_t i = 0; i < 2000; ++i)
    {
        ISOTP_Proccess(ctx_p);

        if (got_callback)
        {
            got_callback = false;
            break;
        }
    }
}

static void Proccess(struct isotp_ctx_t *ctx_p, uint32_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        ISOTP_Proccess(ctx_p);
    }
}

static void FillBuffer(uint8_t *buffer_p, size_t length)
{
    for (size_t i = 0; i < length; ++i)
    {
        buffer_p[i] = i % UINT8_MAX;
    }
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_ISOTP_Bind_InvalidParameters(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);

    uint8_t rx_buffer[8];
    expect_assert_failure(ISOTP_Bind(NULL, rx_buffer, sizeof(rx_buffer), 0x1, 0x2, 0, MockRxStatusHandler, MockTxStatusHandler));
    expect_assert_failure(ISOTP_Bind(&ctx, NULL, sizeof(rx_buffer), 0x1, 0x2, 0, MockRxStatusHandler, MockTxStatusHandler));
    expect_assert_failure(ISOTP_Bind(&ctx, rx_buffer, 0, 0x1, 0x2, 0, MockRxStatusHandler, MockTxStatusHandler));
}

static void test_ISOTP_Process_InvalidParameters(void **state)
{
    expect_assert_failure(ISOTP_Proccess(NULL));
}

static void test_ISOTP_Receive_InvalidParameters(void **state)
{
    uint8_t rx_data[8];
    expect_assert_failure(ISOTP_Receive(NULL, rx_data, sizeof(rx_data)));
    expect_assert_failure(ISOTP_Receive(&ctx, NULL, sizeof(rx_data)));
}

static void test_ISOTP_Send_InvalidParameters(void **state)
{
    uint8_t tx_data[8];
    expect_assert_failure(ISOTP_Send(NULL, tx_data, sizeof(tx_data)));
    expect_assert_failure(ISOTP_Send(&ctx, NULL, sizeof(tx_data)));
}

static void test_ISOTP_SingleFrame(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    expect_any_always(CANInterface_AddFilter, id);
    expect_any_always(CANInterface_AddFilter, mask);
    will_return_maybe(CANInterface_Transmit, true);
    will_return_maybe(SysTime_GetSystemTime, 0);
    will_return_maybe(SysTime_GetDifference, 1);

    uint8_t rx_buffer[32];
    ISOTP_Bind(&ctx, rx_buffer, sizeof(rx_buffer), 0x1, 0x2, 0, MockRxStatusHandler, MockTxStatusHandler);
    ISOTP_Proccess(&ctx);

    uint8_t tx_data[] = {1, 2, 3, 4, 5, 6};
    assert_true(ISOTP_Send(&ctx, tx_data, sizeof(tx_data)));

    ProccessUntilStatus(&ctx, ISOTP_STATUS_DONE);

    uint8_t rx_data[8];
    size_t res = ISOTP_Receive(&ctx, rx_data, sizeof(rx_data));
    assert_int_equal(res, sizeof(tx_data));
    assert_memory_equal(rx_data, tx_data, res);
}

static void test_ISOTP_SingleFrameOverflow(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    expect_any_always(CANInterface_AddFilter, id);
    expect_any_always(CANInterface_AddFilter, mask);
    will_return_maybe(CANInterface_Transmit, true);
    will_return_maybe(SysTime_GetSystemTime, 0);
    will_return_maybe(SysTime_GetDifference, 1);

    uint8_t rx_buffer[4];
    ISOTP_Bind(&ctx, rx_buffer, sizeof(rx_buffer), 0x1, 0x2, 0, MockRxStatusHandler, MockTxStatusHandler);

    uint8_t tx_data[] = {1, 2, 3, 4, 5, 6};
    assert_true(ISOTP_Send(&ctx, tx_data, sizeof(tx_data)));
    ProccessUntilStatus(&ctx, ISOTP_STATUS_OVERFLOW_ABORT);
}

static void test_ISOTP_FirstFrameOverflow(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    expect_any_always(CANInterface_AddFilter, id);
    expect_any_always(CANInterface_AddFilter, mask);
    will_return_maybe(CANInterface_Transmit, true);
    will_return_maybe(SysTime_GetSystemTime, 0);
    will_return_maybe(SysTime_GetDifference, 1);

    uint8_t rx_buffer[4];
    ISOTP_Bind(&ctx, rx_buffer, sizeof(rx_buffer), 0x1, 0x2, 0, MockRxStatusHandler, MockTxStatusHandler);

    uint8_t tx_data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    assert_true(ISOTP_Send(&ctx, tx_data, sizeof(tx_data)));

    expect_value(MockTxStatusHandler, status, ISOTP_STATUS_OVERFLOW_ABORT);
    ProccessUntilStatus(&ctx, ISOTP_STATUS_OVERFLOW_ABORT);
}

static void test_ISOTP_MulipleFrames(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    expect_any_always(CANInterface_AddFilter, id);
    expect_any_always(CANInterface_AddFilter, mask);
    will_return_maybe(CANInterface_Transmit, true);
    will_return_maybe(SysTime_GetSystemTime, 0);
    will_return_maybe(SysTime_GetDifference, 1);

    uint8_t rx_buffer[32];
    ISOTP_Bind(&ctx, rx_buffer, sizeof(rx_buffer), 0x1, 0x2, 0, MockRxStatusHandler, MockTxStatusHandler);

    uint8_t tx_data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    assert_true(ISOTP_Send(&ctx, tx_data, sizeof(tx_data)));

    expect_value(MockTxStatusHandler, status, ISOTP_STATUS_DONE);
    ProccessUntilStatus(&ctx, ISOTP_STATUS_DONE);

    uint8_t rx_data[32];
    size_t res = ISOTP_Receive(&ctx, rx_data, sizeof(rx_data));
    assert_int_equal(res, sizeof(tx_data));
    assert_memory_equal(rx_data, tx_data, res);
}

static void test_ISOTP_MaxDataLength(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    expect_any_always(CANInterface_AddFilter, id);
    expect_any_always(CANInterface_AddFilter, mask);
    will_return_maybe(CANInterface_Transmit, true);
    will_return_maybe(SysTime_GetSystemTime, 0);
    will_return_maybe(SysTime_GetDifference, 1);

    const size_t isotp_max_data_length = 4095;
    uint8_t rx_buffer[isotp_max_data_length];
    ISOTP_Bind(&ctx, rx_buffer, sizeof(rx_buffer), 0x1, 0x2, 0, MockRxStatusHandler, MockTxStatusHandler);

    uint8_t tx_data[isotp_max_data_length];
    FillBuffer(tx_data, sizeof(tx_data));
    assert_true(ISOTP_Send(&ctx, tx_data, sizeof(tx_data)));

    expect_value(MockTxStatusHandler, status, ISOTP_STATUS_DONE);
    ProccessUntilStatus(&ctx, ISOTP_STATUS_DONE);

    uint8_t rx_data[isotp_max_data_length];
    size_t res = ISOTP_Receive(&ctx, rx_data, sizeof(rx_data));
    assert_int_equal(res, sizeof(tx_data));
    assert_memory_equal(rx_data, tx_data, res);
}

static void test_ISOTP_ConsecutiveFrameTimeout(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    expect_any_always(CANInterface_AddFilter, id);
    expect_any_always(CANInterface_AddFilter, mask);
    will_return_maybe(CANInterface_Transmit, true);

    /* Should cause timeout while waiting for the first consecutive frame. */
    will_return_maybe(SysTime_GetSystemTime, 0);
    will_return_maybe(SysTime_GetDifference, 1001);

    uint8_t rx_buffer[32];
    ISOTP_Bind(&ctx, rx_buffer, sizeof(rx_buffer), 0x1, 0x2, 0, MockRxStatusHandler, MockTxStatusHandler);

    uint8_t tx_data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    assert_true(ISOTP_Send(&ctx, tx_data, sizeof(tx_data)));

    expect_value(MockTxStatusHandler, status, ISOTP_STATUS_DONE);
    ProccessUntilStatus(&ctx, ISOTP_STATUS_TIMEOUT);

    uint8_t rx_data[32];
    size_t res = ISOTP_Receive(&ctx, rx_data, sizeof(rx_data));
    assert_int_equal(res, 0);
}

static void test_ISOTP_FlowControlFrameTimeout(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    expect_any_always(CANInterface_AddFilter, id);
    expect_any_always(CANInterface_AddFilter, mask);
    will_return_maybe(CANInterface_Transmit, true);

    /* Should cause timeout while waiting for the first flow control frame. */
    will_return_maybe(SysTime_GetSystemTime, 0);
    will_return_maybe(SysTime_GetDifference, 1001);

    uint8_t rx_buffer[32];
    ISOTP_Bind(&ctx, rx_buffer, sizeof(rx_buffer), 0x1, 0x2, 0, MockRxStatusHandler, MockTxStatusHandler);

    /* Drop the First frame so that the receiver does not send a Flow control frame in response.*/
    drop_frame = true;
    uint8_t tx_data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    assert_true(ISOTP_Send(&ctx, tx_data, sizeof(tx_data)));

    expect_value(MockTxStatusHandler, status, ISOTP_STATUS_TIMEOUT);
    Proccess(&ctx, 1);
}

static void test_ISOTP_ConsecutiveFrameLost(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    expect_any_always(CANInterface_AddFilter, id);
    expect_any_always(CANInterface_AddFilter, mask);
    will_return_maybe(CANInterface_Transmit, true);
    will_return_maybe(SysTime_GetSystemTime, 0);
    will_return_maybe(SysTime_GetDifference, 1);

    uint8_t rx_buffer[64];
    ISOTP_Bind(&ctx, rx_buffer, sizeof(rx_buffer), 0x1, 0x2, 0, MockRxStatusHandler, MockTxStatusHandler);

    uint8_t tx_data[64];
    FillBuffer(tx_data, sizeof(tx_data));
    assert_true(ISOTP_Send(&ctx, tx_data, sizeof(tx_data)));

    expect_value(MockRxStatusHandler, status, ISOTP_STATUS_LOST_FRAME);
    Proccess(&ctx, 2);
    drop_frame = true;
    Proccess(&ctx, 5);

    uint8_t rx_data[32];
    size_t res = ISOTP_Receive(&ctx, rx_data, sizeof(rx_data));
    assert_int_equal(res, 0);
}

static void test_ISOTP_SendWithActiveTransfer(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    expect_any_always(CANInterface_AddFilter, id);
    expect_any_always(CANInterface_AddFilter, mask);
    will_return_maybe(CANInterface_Transmit, true);
    will_return_maybe(SysTime_GetSystemTime, 0);
    will_return_maybe(SysTime_GetDifference, 1);

    uint8_t rx_buffer[32];
    ISOTP_Bind(&ctx, rx_buffer, sizeof(rx_buffer), 0x1, 0x2, 0, MockRxStatusHandler, MockTxStatusHandler);

    uint8_t tx_data_1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    uint8_t tx_data_2[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    assert_true(ISOTP_Send(&ctx, tx_data_1, sizeof(tx_data_1)));
    assert_false(ISOTP_Send(&ctx, tx_data_2, sizeof(tx_data_2)));

    expect_value(MockTxStatusHandler, status, ISOTP_STATUS_DONE);
    ProccessUntilStatus(&ctx, ISOTP_STATUS_DONE);

    uint8_t rx_data[32];
    size_t res = ISOTP_Receive(&ctx, rx_data, sizeof(rx_data));
    assert_int_equal(res, sizeof(tx_data_1));
    assert_memory_equal(rx_data, tx_data_1, res);
}

static void test_ISOTP_SendWithCanTransmitError(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    expect_any_always(CANInterface_AddFilter, id);
    expect_any_always(CANInterface_AddFilter, mask);
    will_return_maybe(CANInterface_Transmit, false);
    will_return_maybe(SysTime_GetSystemTime, 0);
    will_return_maybe(SysTime_GetDifference, 1);

    uint8_t rx_buffer[32];
    ISOTP_Bind(&ctx, rx_buffer, sizeof(rx_buffer), 0x1, 0x2, 0, MockRxStatusHandler, MockTxStatusHandler);

    uint8_t tx_data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    assert_false(ISOTP_Send(&ctx, tx_data, sizeof(tx_data)));
}

static void test_ISOTP_WaitingForRxSpace(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    expect_any_always(CANInterface_AddFilter, id);
    expect_any_always(CANInterface_AddFilter, mask);
    will_return_maybe(CANInterface_Transmit, true);
    will_return_maybe(SysTime_GetSystemTime, 0);
    will_return_maybe(SysTime_GetDifference, 101);

    uint8_t rx_buffer[15];
    ISOTP_Bind(&ctx, rx_buffer, sizeof(rx_buffer), 0x1, 0x2, 0, MockRxStatusHandler, MockTxStatusHandler);

    uint8_t tx_data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    assert_true(ISOTP_Send(&ctx, tx_data, sizeof(tx_data)));

    uint8_t rx_data[16];
    size_t res;
    ProccessUntilStatus(&ctx, ISOTP_STATUS_WAITING);
    res = ISOTP_Receive(&ctx, rx_data, sizeof(rx_data));
    expect_value(MockTxStatusHandler, status, ISOTP_STATUS_DONE);
    ProccessUntilStatus(&ctx, ISOTP_STATUS_DONE);
    res += ISOTP_Receive(&ctx, rx_data + res, sizeof(rx_data) - res);
    assert_int_equal(res, sizeof(tx_data));
    assert_memory_equal(rx_data, tx_data, res);
}

static void test_ISOTP_RxWaitingTimeout(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    expect_any_always(CANInterface_AddFilter, id);
    expect_any_always(CANInterface_AddFilter, mask);
    will_return_maybe(CANInterface_Transmit, true);
    will_return_maybe(SysTime_GetSystemTime, 0);
    will_return_maybe(SysTime_GetDifference, 101);

    uint8_t rx_buffer[15];
    ISOTP_Bind(&ctx, rx_buffer, sizeof(rx_buffer), 0x1, 0x2, 0, MockRxStatusHandler, MockTxStatusHandler);

    uint8_t tx_data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    assert_true(ISOTP_Send(&ctx, tx_data, sizeof(tx_data)));

    ProccessUntilStatus(&ctx, ISOTP_STATUS_WAITING);
    expect_value(MockTxStatusHandler, status, ISOTP_STATUS_OVERFLOW_ABORT);
    ProccessUntilStatus(&ctx, ISOTP_STATUS_TIMEOUT);

    uint8_t rx_data[32];
    size_t res = ISOTP_Receive(&ctx, rx_data, sizeof(rx_data));
    assert_int_equal(res, 0);
}

static void test_ISOTP_TxWaitingTimeout(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    expect_any_always(CANInterface_AddFilter, id);
    expect_any_always(CANInterface_AddFilter, mask);
    will_return_maybe(CANInterface_Transmit, true);
    will_return_maybe(SysTime_GetSystemTime, 0);
    will_return_maybe(SysTime_GetDifference, 101);

    uint8_t rx_buffer[15];
    ISOTP_Bind(&ctx, rx_buffer, sizeof(rx_buffer), 0x1, 0x2, 0, MockRxStatusHandler, MockTxStatusHandler);

    uint8_t tx_data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    assert_true(ISOTP_Send(&ctx, tx_data, sizeof(tx_data)));

    ProccessUntilStatus(&ctx, ISOTP_STATUS_WAITING);
    Proccess(&ctx, 7);

    /**
     * Inject extra flow control frames with flag=WAIT to trigger timeout on the tx side.
     * This is needed since the same hard-coded max wf count is used for rx and tx, otherwise
     * the rx side will timeout and send abort before the tx side can timeout.
     */
    InjectFlowControlFrame(0x02);
    InjectFlowControlFrame(0x02);

    expect_value(MockTxStatusHandler, status, ISOTP_STATUS_TIMEOUT);
    ProccessUntilStatus(&ctx, ISOTP_STATUS_TIMEOUT);
}

static void test_ISOTP_SeparationTime(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    expect_any_always(CANInterface_AddFilter, id);
    expect_any_always(CANInterface_AddFilter, mask);
    will_return_maybe(CANInterface_Transmit, true);
    will_return_maybe(SysTime_GetSystemTime, 0);
    will_return_maybe(SysTime_GetDifference, 1);

    uint8_t rx_buffer[8];
    const uint8_t tx_data[] = {0, 1, 2, 3, 4, 5, 6, 7};

    /* Millisecond range */
    const uint8_t separation_times_ms[] = {0, 1, 60, 126, 127};
    for (size_t i = 0; i < ElementsIn(separation_times_ms); ++i)
    {
        ISOTP_Bind(&ctx, rx_buffer, sizeof(rx_buffer), 0x1, 0x2, separation_times_ms[i], MockRxStatusHandler, MockTxStatusHandler);

        assert_true(ISOTP_Send(&ctx, tx_data, sizeof(tx_data)));
        expect_value(MockTxStatusHandler, status, ISOTP_STATUS_DONE);

        ProccessUntilStatus(&ctx, ISOTP_STATUS_DONE);
        assert_int_equal(ctx.tx_link.base.separation_time, separation_times_ms[i] * 1000);
    }

    /* Microseconds range */
    const uint8_t separation_times_us[] = {241, 242, 245, 248, 249};
    for (size_t i = 0; i < ElementsIn(separation_times_us); ++i)
    {
        ISOTP_Bind(&ctx, rx_buffer, sizeof(rx_buffer), 0x1, 0x2, separation_times_us[i], MockRxStatusHandler, MockTxStatusHandler);

        assert_true(ISOTP_Send(&ctx, tx_data, sizeof(tx_data)));
        expect_value(MockTxStatusHandler, status, ISOTP_STATUS_DONE);

        ProccessUntilStatus(&ctx, ISOTP_STATUS_DONE);
        /* Microseconds timer is not implemented yet so always expect 1 ms. */
        assert_int_equal(ctx.tx_link.base.separation_time, (separation_times_us[i] - 240)* 100);
    }

    /* Invalid range */
    const uint8_t separation_times_invalid[] = {128, 240, 250, UINT8_MAX};
    for (size_t i = 0; i < ElementsIn(separation_times_invalid); ++i)
    {
        ISOTP_Bind(&ctx, rx_buffer, sizeof(rx_buffer), 0x1, 0x2, separation_times_invalid[i], MockRxStatusHandler, MockTxStatusHandler);

        assert_true(ISOTP_Send(&ctx, tx_data, sizeof(tx_data)));
        expect_value(MockTxStatusHandler, status, ISOTP_STATUS_DONE);

        ProccessUntilStatus(&ctx, ISOTP_STATUS_DONE);
        /* Expect default value (10 ms) when separation time is invalid. */
        assert_int_equal(ctx.tx_link.base.separation_time, 10000);
    }
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_ISOTP[] =
    {
        cmocka_unit_test_setup(test_ISOTP_Bind_InvalidParameters, Setup),
        cmocka_unit_test_setup(test_ISOTP_Process_InvalidParameters, Setup),
        cmocka_unit_test_setup(test_ISOTP_Receive_InvalidParameters, Setup),
        cmocka_unit_test_setup(test_ISOTP_Send_InvalidParameters, Setup),
        cmocka_unit_test_setup(test_ISOTP_SingleFrame, Setup),
        cmocka_unit_test_setup(test_ISOTP_SingleFrameOverflow, Setup),
        cmocka_unit_test_setup(test_ISOTP_FirstFrameOverflow, Setup),
        cmocka_unit_test_setup(test_ISOTP_MulipleFrames, Setup),
        cmocka_unit_test_setup(test_ISOTP_MaxDataLength, Setup),
        cmocka_unit_test_setup(test_ISOTP_ConsecutiveFrameTimeout, Setup),
        cmocka_unit_test_setup(test_ISOTP_ConsecutiveFrameLost, Setup),
        cmocka_unit_test_setup(test_ISOTP_FlowControlFrameTimeout, Setup),
        cmocka_unit_test_setup(test_ISOTP_SendWithActiveTransfer, Setup),
        cmocka_unit_test_setup(test_ISOTP_SendWithCanTransmitError, Setup),
        cmocka_unit_test_setup(test_ISOTP_WaitingForRxSpace, Setup),
        cmocka_unit_test_setup(test_ISOTP_RxWaitingTimeout, Setup),
        cmocka_unit_test_setup(test_ISOTP_TxWaitingTimeout, Setup),
        cmocka_unit_test_setup(test_ISOTP_SeparationTime, Setup),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_ISOTP, NULL, NULL);
}
