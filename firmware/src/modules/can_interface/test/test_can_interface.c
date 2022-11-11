/**
 * @file   test_can_interface.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the CANInterface module.
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

#include <libopencm3/stm32/can.h>
#include "can_interface.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

extern void usb_lp_can_rx0_isr(void);

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct logging_logger_t *dummy_logger;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static int Setup(void **state)
{
    will_return(Logging_GetLogger, dummy_logger);
    will_return(can_init, 0);
    expect_value(can_init, canport, CAN1);
    expect_value(can_reset, canport, CAN1);
    expect_value(can_enable_irq, canport, CAN1);
    expect_value(can_enable_irq, irq, CAN_IER_FMPIE0);
    CANInterface_Init();

    return 0;
}

static void Listener(const struct can_frame_t *frame_p, void *arg_p)
{
    check_expected(frame_p->id);
    check_expected(frame_p->size);
    check_expected(frame_p->data);

    if (arg_p != NULL)
    {
        check_expected_ptr(arg_p);
    }
}

static void ReceiveCANFrame(const struct can_frame_t *frame_p)
{
    expect_value(can_receive, canport, CAN1);
    will_return(can_receive, frame_p->id);
    will_return(can_receive, frame_p->size);
    will_return(can_receive, frame_p->data);
    usb_lp_can_rx0_isr();
}

static uint16_t ShiftedIDMask(uint16_t id_mask)
{
    uint16_t result = id_mask;

    if (id_mask != 0xFFFF)
    {
        result = id_mask << 5;
    }
    return result;
}

static void ExpectCANFilterInit(uint32_t filter_id, uint16_t id1, uint16_t mask1, uint16_t id2, uint16_t mask2)
{
    expect_value(can_filter_id_mask_16bit_init, nr, filter_id);
    expect_value(can_filter_id_mask_16bit_init, id1, ShiftedIDMask(id1));
    expect_value(can_filter_id_mask_16bit_init, mask1, ShiftedIDMask(mask1));
    expect_value(can_filter_id_mask_16bit_init, id2, ShiftedIDMask(id2));
    expect_value(can_filter_id_mask_16bit_init, mask2, ShiftedIDMask(mask2));
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_CANInterface_Init_Error(void **state)
{
    will_return(Logging_GetLogger, dummy_logger);
    will_return(can_init, 1);
    expect_value(can_init, canport, CAN1);
    expect_value(can_reset, canport, CAN1);
    expect_assert_failure(CANInterface_Init())
}

static void test_CANInterface_Init(void **state)
{
    will_return(Logging_GetLogger, dummy_logger);
    will_return(can_init, 0);
    expect_value(can_init, canport, CAN1);
    expect_value(can_reset, canport, CAN1);
    expect_value(can_enable_irq, canport, CAN1);
    expect_value(can_enable_irq, irq, CAN_IER_FMPIE0);
    CANInterface_Init();
}

static void test_CANInterface_RegisterListener_Invalid(void **state)
{
    expect_assert_failure(CANInterface_RegisterListener(NULL, NULL))
}

static void test_CANInterface_RegisterListener_Full(void **state)
{
    const size_t max_number_of_listeners = 5;
    for (size_t i = 0; i < max_number_of_listeners; ++i)
    {
        CANInterface_RegisterListener(Listener, NULL);
    }

    expect_assert_failure(CANInterface_RegisterListener(Listener, NULL));
}

static void test_CANInterface_ReceiveWithNoListeners(void **state)
{
    struct can_frame_t frame = {.id = 0x1, .size = 2, .data = {0x3, 0x4}};
    ReceiveCANFrame(&frame);
}

static void test_CANInterface_ReceiveWithListener(void **state)
{
    const struct can_frame_t frame = {.id = 0x1, .size = 2, .data = {0x3, 0x4}};

    expect_value(Listener, frame_p->id, frame.id);
    expect_value(Listener, frame_p->size,frame.size);
    expect_memory(Listener, frame_p->data, frame.data, frame.size);

    CANInterface_RegisterListener(Listener, NULL);
    ReceiveCANFrame(&frame);
}

static void test_CANInterface_Transmit_Invalid(void **state)
{
    const uint32_t id = 0x1;
    uint8_t data;
    expect_assert_failure(CANInterface_Transmit(id, NULL, 1));
    expect_assert_failure(CANInterface_Transmit(id, &data, 9));
    expect_assert_failure(CANInterface_Transmit(id, NULL, 9));
}

static void test_CANInterface_Transmit_Error(void **state)
{
    const uint32_t id = 0x2;
    uint8_t data = 1;

    will_return_maybe(SysTime_GetSystemTime, 0);
    will_return_maybe(can_available_mailbox, true);
    expect_value(can_transmit, canport, CAN1);
    expect_value(can_transmit, id, id);
    expect_value(can_transmit, length, sizeof(data));
    expect_memory(can_transmit, data, &data, sizeof(data));
    will_return(can_transmit, -1);

    assert_false(CANInterface_Transmit(id, &data, sizeof(data)));
}

static void test_CANInterface_Transmit_Timeout(void **state)
{
    const uint32_t id = 0x2;
    uint8_t data = 1;
    const uint32_t timeout_ms = 2;

    will_return_maybe(SysTime_GetSystemTime, 0);
    will_return_maybe(can_available_mailbox, false);
    for (uint32_t i = 0; i <= timeout_ms; ++i)
    {
        will_return(SysTime_GetDifference, i);
    }
    expect_value(can_transmit, canport, CAN1);
    expect_value(can_transmit, id, id);
    expect_value(can_transmit, length, sizeof(data));
    expect_memory(can_transmit, data, &data, sizeof(data));
    will_return(can_transmit, -1);

    assert_false(CANInterface_Transmit(id, &data, sizeof(data)));
}

static void test_CANInterface_Transmit(void **state)
{
    const uint32_t id = 0x3;
    uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7};
    int mailbox_number = 0;

    will_return_maybe(SysTime_GetSystemTime, 0);
    will_return_maybe(can_available_mailbox, true);
    expect_value(can_transmit, canport, CAN1);
    expect_value(can_transmit, id, id);
    expect_value(can_transmit, length, sizeof(data));
    expect_memory(can_transmit, data, &data, sizeof(data));
    will_return(can_transmit, mailbox_number);

    assert_true(CANInterface_Transmit(id, &data, sizeof(data)));
}

static void test_CANInterface_AddFilter(void **state)
{
    const uint16_t default_id_mask = 0xFFFF;

    const uint16_t id1 = 0xA0;
    const uint16_t mask1 = 0xF0;
    const uint16_t id2 = 0xB0;
    const uint16_t mask2 = 0x0F;
    uint32_t filter_id = 0;

    /* First filter on filter bank 0 */
    ExpectCANFilterInit(filter_id, id1, mask1, default_id_mask, default_id_mask);
    CANInterface_AddFilter(id1, mask1);

    /* Second filter on filter bank 0 */
    ExpectCANFilterInit(filter_id, id1, mask1, id2, mask2);
    CANInterface_AddFilter(id2, mask2);

    /* First filter on filter bank 1 */
    filter_id = 1;
    ExpectCANFilterInit(filter_id, id1, mask1, default_id_mask, default_id_mask);
    CANInterface_AddFilter(id1, mask1);

    /* Second filter on filter bank 1 */
    ExpectCANFilterInit(filter_id, id1, mask1, id2, mask2);
    CANInterface_AddFilter(id2, mask2);

    /* Use all remaining filter banks and expect assert when no more banks are available. */
    const size_t max_number_of_filters = 28;
    const size_t number_of_used_filters = 4;
    for (size_t i = 0; i < (max_number_of_filters - number_of_used_filters); ++i)
    {
        expect_any(can_filter_id_mask_16bit_init, nr);
        expect_any(can_filter_id_mask_16bit_init, id1);
        expect_any(can_filter_id_mask_16bit_init, mask1);
        expect_any(can_filter_id_mask_16bit_init, id2);
        expect_any(can_filter_id_mask_16bit_init, mask2);
        CANInterface_AddFilter(id1, mask1);
    }
    expect_assert_failure(CANInterface_AddFilter(id1, mask1));
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_FIFO[] =
    {
        cmocka_unit_test(test_CANInterface_Init_Error),
        cmocka_unit_test(test_CANInterface_Init),
        cmocka_unit_test_setup(test_CANInterface_RegisterListener_Invalid, Setup),
        cmocka_unit_test_setup(test_CANInterface_RegisterListener_Full, Setup),
        cmocka_unit_test_setup(test_CANInterface_ReceiveWithNoListeners, Setup),
        cmocka_unit_test_setup(test_CANInterface_ReceiveWithListener, Setup),
        cmocka_unit_test_setup(test_CANInterface_Transmit_Invalid, Setup),
        cmocka_unit_test_setup(test_CANInterface_Transmit_Error, Setup),
        cmocka_unit_test_setup(test_CANInterface_Transmit_Timeout, Setup),
        cmocka_unit_test_setup(test_CANInterface_Transmit, Setup),
        cmocka_unit_test_setup(test_CANInterface_AddFilter, Setup),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_FIFO, NULL, NULL);
}
