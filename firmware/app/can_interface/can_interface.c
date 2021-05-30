/**
 * @file   can_interface.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  CAN-Bus interface.
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

#include <stddef.h>
#include <assert.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/can.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include "utility.h"
#include "logging.h"
#include "candb.h"
#include "can_interface.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define CANIF_LOGGER_NAME "CANIf"
#ifndef CANIF_LOGGER_DEBUG_LEVEL
#define CANIF_LOGGER_DEBUG_LEVEL LOGGING_DEBUG
#endif

#define MAX_NUMBER_OF_LISTENERS 1

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct module_t
{
    logging_logger_t logger;
    caninterface_listener_cb_t listeners[MAX_NUMBER_OF_LISTENERS];
    size_t number_of_listeners;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct module_t module;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static void InitCANPeripheral(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void CANInterface_Init(void)
{
    module = (__typeof__(module)) {0};
    module.logger = Logging_GetLogger(CANIF_LOGGER_NAME);
    Logging_SetLevel(module.logger, CANIF_LOGGER_DEBUG_LEVEL);

    InitCANPeripheral();
    Logging_Info(module.logger, "CAN initialized");
}

bool CANInterface_Transmit(uint32_t id, void *data_p, size_t size)
{
    assert(data_p != NULL);
    assert(size <= 8);

    Logging_Debug(module.logger, "CANTX{id=0x%x}", id);

    bool extended_id = false;
    bool request_transmit = false;
    return can_transmit(CAN1,
                        id,
                        extended_id,
                        request_transmit,
                        size,
                        data_p) != -1;
}

void CANInterface_RegisterListener(caninterface_listener_cb_t listener_cb)
{
    assert(listener_cb != NULL);
    assert(module.number_of_listeners < ElementsIn(module.listeners));

    module.listeners[module.number_of_listeners] = listener_cb;
    ++module.number_of_listeners;

    Logging_Info(module.logger, "New listener registered: 0x%x", (int)listener_cb);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static void InitCANPeripheral(void)
{
    Logging_Debug(module.logger, "%s()", __func__);

    rcc_periph_clock_enable(RCC_AFIO);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_CAN1);

    AFIO_MAPR |= AFIO_MAPR_CAN1_REMAP_PORTB;

    gpio_set_mode(GPIO_BANK_CAN1_PB_RX, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO_CAN1_PB_RX);
    gpio_set(GPIO_BANK_CAN1_PB_RX, GPIO_CAN1_PB_RX);
    gpio_set_mode(GPIO_BANK_CAN1_PB_TX, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_CAN1_PB_TX);

    nvic_enable_irq(NVIC_USB_LP_CAN_RX0_IRQ);
    nvic_set_priority(NVIC_USB_LP_CAN_RX0_IRQ, 1);

    can_reset(CAN1);

    const bool time_triggered_com_mode = false;
    const bool automatic_bus_off_management = true;
    const bool automatic_wakeup_mode = false;
    const bool no_automatic_retransmission = false;
    const bool receive_fifo_locked_mode = false;
    const bool transmit_fifo_priority = false;
    const bool loopback = false;
    const bool silent = false;

    if (can_init(CAN1,
                 time_triggered_com_mode,
                 automatic_bus_off_management,
                 automatic_wakeup_mode,
                 no_automatic_retransmission,
                 receive_fifo_locked_mode,
                 transmit_fifo_priority,
                 CAN_BTR_SJW_1TQ,
                 CAN_BTR_TS1_9TQ,
                 CAN_BTR_TS2_6TQ,
                 2,
                 loopback,
                 silent))
    {
        Logging_Critical(module.logger, "Failed to initialize CAN1");
        assert(false);
    }

    const uint32_t filter_id = 0;
    const uint32_t can_id = 0;
    const uint32_t can_mask = 0;
    const uint32_t fifo = 0;
    const bool enable = true;
    can_filter_id_mask_32bit_init(filter_id, can_id, can_mask, fifo, enable);

    /* Enable CAN RX interrupt. */
    can_enable_irq(CAN1, CAN_IER_FMPIE0);
}

void NotifyListeners(const struct can_frame_t *frame_p)
{
    for (size_t i = 0; i < module.number_of_listeners; ++i)
    {
        module.listeners[i](frame_p);
    }
}

//////////////////////////////////////////////////////////////////////////
//ISR
//////////////////////////////////////////////////////////////////////////

void usb_lp_can_rx0_isr(void)
{
    bool ext;
    bool rtr;
    uint8_t fmi;
    struct can_frame_t frame;

    can_receive(CAN1, 0, false, &frame.id, &ext, &rtr, &fmi, &frame.size, frame.data, NULL);
    can_fifo_release(CAN1, 0);

    Logging_Debug(module.logger, "CANRX{id=0x%x}", frame.id);
    NotifyListeners(&frame);
}
