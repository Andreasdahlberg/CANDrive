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

void Listener(const struct can_frame_t *frame_p)
{
    check_expected(frame_p->id);
    check_expected(frame_p->size);
    check_expected(frame_p->data);
}

static void ReceiveCANFrame(const struct can_frame_t *frame_p)
{
    expect_value(can_receive, canport, CAN1);
    will_return(can_receive, frame_p->id);
    will_return(can_receive, frame_p->size);
    will_return(can_receive, frame_p->data);
    usb_lp_can_rx0_isr();
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

void test_CANInterface_Init_Error(void **state)
{
    will_return(Logging_GetLogger, dummy_logger);
    will_return(can_init, 1);
    expect_value(can_init, canport, CAN1);
    expect_value(can_reset, canport, CAN1);
    expect_assert_failure(CANInterface_Init())
}

void test_CANInterface_Init(void **state)
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
    expect_assert_failure(CANInterface_RegisterListener(NULL))
}

static void test_CANInterface_RegisterListener_Full(void **state)
{
    const size_t max_number_of_listeners = 1;
    for (size_t i = 0; i < max_number_of_listeners; ++i)
    {
        CANInterface_RegisterListener(Listener);
    }

    expect_assert_failure(CANInterface_RegisterListener(Listener));
}

void test_CANInterface_ReceiveWithNoListeners(void **state)
{
    struct can_frame_t frame = {.id = 0x1, .size = 2, .data = {0x3, 0x4}};
    ReceiveCANFrame(&frame);
}


void test_CANInterface_ReceiveWithListener(void **state)
{
    const struct can_frame_t frame = {.id = 0x1, .size = 2, .data = {0x3, 0x4}};

    expect_value(Listener, frame_p->id, frame.id);
    expect_value(Listener, frame_p->size,frame.size);
    expect_memory(Listener, frame_p->data, frame.data, frame.size);

    CANInterface_RegisterListener(Listener);
    ReceiveCANFrame(&frame);
}

void test_CANInterface_Transmit_Invalid(void **state)
{
    const uint32_t id = 0x1;
    uint8_t data;
    expect_assert_failure(CANInterface_Transmit(id, NULL, 1));
    expect_assert_failure(CANInterface_Transmit(id, &data, 9));
    expect_assert_failure(CANInterface_Transmit(id, NULL, 9));
}

void test_CANInterface_Transmit_Error(void **state)
{
    const uint32_t id = 0x2;
    uint8_t data = 1;

    expect_value(can_transmit, canport, CAN1);
    expect_value(can_transmit, id, id);
    expect_value(can_transmit, length, sizeof(data));
    expect_memory(can_transmit, data, &data, sizeof(data));
    will_return(can_transmit, -1);

    assert_false(CANInterface_Transmit(id, &data, sizeof(data)));
}

void test_CANInterface_Transmit(void **state)
{
    const uint32_t id = 0x3;
    uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7};
    int mailbox_number = 0;

    expect_value(can_transmit, canport, CAN1);
    expect_value(can_transmit, id, id);
    expect_value(can_transmit, length, sizeof(data));
    expect_memory(can_transmit, data, &data, sizeof(data));
    will_return(can_transmit, mailbox_number);

    assert_true(CANInterface_Transmit(id, &data, sizeof(data)));
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
        cmocka_unit_test_setup(test_CANInterface_Transmit, Setup),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_FIFO, NULL, NULL);
}
