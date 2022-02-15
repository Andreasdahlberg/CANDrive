/**
 * @file   isotp.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  ISO15765-2/ISO-TP module.
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
#include "logging.h"
#include "systime.h"
#include "isotp.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define ISOTP_LOGGER_NAME "ISOTP"
#ifndef ISOTP_LOGGER_DEBUG_LEVEL
#define ISOTP_LOGGER_DEBUG_LEVEL LOGGING_INFO
#endif

#define SF_DATA_LENGTH 7
#define FF_DATA_LENGTH 6
#define CF_DATA_LENGTH 7
#define WF_MAX 10
#define CF_TIMEOUT_MS 1000
#define FC_TIMEOUT_MS 1000

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

enum isotp_frame_type_t
{
    ISOTP_SINGLE_FRAME = 0,
    ISOTP_FIRST_FRAME,
    ISOTP_CONSECUTIVE_FRAME,
    ISOTP_FLOW_CONTROL_FRAME,
    ISOTP_FRAME_TYPE_END
};

enum isotp_flow_control_flag_t
{
    ISOTP_FC_CONTINUE_TO_SEND = 0,
    ISOTP_FC_WAIT,
    ISOTP_FC_OVERFLOW_ABORT
};

struct isotp_sf_t
{
    uint8_t size : 4;
    uint8_t type : 4;
    uint8_t data[SF_DATA_LENGTH];
};

struct isotp_ff_t
{
    uint8_t size_high : 4;
    uint8_t type : 4;
    uint8_t size_low;
    uint8_t data[FF_DATA_LENGTH];
};
struct isotp_cf_t
{
    uint8_t index : 4;
    uint8_t type : 4;
    uint8_t data[CF_DATA_LENGTH];
};

struct isotp_fc_t
{
    uint8_t flag : 4;
    uint8_t type : 4;
    uint8_t block_size;
    uint8_t st;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static inline void ConfigureRxLink(struct isotp_recv_link_t *link_p, void *rx_buffer_p, size_t rx_buffer_size, uint16_t rx_id, uint16_t tx_id, uint8_t separation_time, isotp_status_callback_t callback_fp);
static inline void ConfigureTxLink(struct isotp_send_link_t *link_p, void *tx_buffer_p, size_t tx_buffer_size, uint16_t rx_id, uint16_t tx_id, isotp_status_callback_t callback_fp);
static void ProccessRxLink(struct isotp_recv_link_t *link_p);
static void ProccessTxLink(struct isotp_send_link_t *link_p);
static void CanListener(const struct can_frame_t *frame_p, void *arg_p);
static inline enum isotp_frame_type_t GetFrameType(const struct can_frame_t *frame_p);
static void HandleSingleFrame(struct isotp_recv_link_t *link_p, const struct can_frame_t *frame_p);
static void HandleFirstFrame(struct isotp_recv_link_t *link_p, const struct can_frame_t *frame_p);
static void CheckForFirstAndSingleFrame(struct isotp_recv_link_t *link_p);
static void SendFlowControlFrame(struct isotp_recv_link_t *link_p, enum isotp_flow_control_flag_t status);
static void CheckForConsecutiveFrame(struct isotp_recv_link_t *link_p);
static void HandleConsecutiveFrame(struct isotp_recv_link_t *link_p, const struct can_frame_t *frame_p);
static void CheckIfReadyForData(struct isotp_recv_link_t *link_p);
static uint8_t GetBlockSize(const struct isotp_recv_link_t *link_p);
static bool SendSingleFrame(const struct isotp_send_link_t *link_p, const void *data_p, size_t length);
static bool SendFirstFrame(struct isotp_send_link_t *link_p, size_t length);
static void CheckIfSeparationTimeHasElapsed(struct isotp_send_link_t *link_p);
static void CheckForFlowControlFrame(struct isotp_send_link_t *link_p);
static void HandleFlowControlFrame(struct isotp_send_link_t *link_p, const struct can_frame_t *frame_p);
static uint32_t SeparationTimeToUs(uint8_t st);
static bool SendConsecutiveFrame(struct isotp_send_link_t *link_p);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void ISOTP_Bind(struct isotp_ctx_t *ctx_p, void *rx_buffer_p, size_t rx_buffer_size, void *tx_buffer_p, size_t tx_buffer_size, uint16_t rx_id, uint16_t tx_id, isotp_status_callback_t rx_callback_fp, isotp_status_callback_t tx_callback_fp)
{
    assert(ctx_p != NULL);

    *ctx_p = (__typeof__(*ctx_p)) {0};
    ctx_p->logger_p = Logging_GetLogger(ISOTP_LOGGER_NAME);
    Logging_SetLevel(ctx_p->logger_p, ISOTP_LOGGER_DEBUG_LEVEL);

    const uint8_t default_separation_time = 0;
    ConfigureRxLink(&ctx_p->rx_link, rx_buffer_p, rx_buffer_size, rx_id, tx_id, default_separation_time, rx_callback_fp);
    ConfigureTxLink(&ctx_p->tx_link, tx_buffer_p, tx_buffer_size, rx_id, tx_id, tx_callback_fp);

    const uint32_t id_mask = 0xffff;
    CANInterface_AddFilter(rx_id, id_mask);
    CANInterface_RegisterListener(CanListener, &ctx_p->rx_link);
    CANInterface_RegisterListener(CanListener, &ctx_p->tx_link);

    Logging_Info(ctx_p->logger_p, "ISO-TP connection initialized: {rx_id: 0x%x, tx_id: 0x%x}", rx_id, tx_id);
}

void ISOTP_SetSeparationTime(struct isotp_ctx_t *ctx_p, uint8_t separation_time)
{
    assert(ctx_p != NULL);
    ctx_p->rx_link.base.separation_time = separation_time;
}

void ISOTP_Proccess(struct isotp_ctx_t *ctx_p)
{
    assert(ctx_p != NULL);

    ProccessRxLink(&ctx_p->rx_link);
    ProccessTxLink(&ctx_p->tx_link);
}

bool ISOTP_Send(struct isotp_ctx_t *ctx_p, const void *data_p, size_t length)
{
    assert(ctx_p != NULL);
    assert(data_p != NULL);

    bool status = false;
    if (ctx_p->tx_link.state == ISOTP_TX_INACTIVE)
    {
        if (length <= SF_DATA_LENGTH)
        {
            /* No need to write data to the TX stream since all data fits in this frame. */
            status = SendSingleFrame(&ctx_p->tx_link, data_p, length);
        }
        else
        {
            status = Stream_Write(&ctx_p->tx_link.tx_stream, data_p, length) == length;
            status = status && SendFirstFrame(&ctx_p->tx_link, length);
            if (status)
            {
                ctx_p->tx_link.state = ISOTP_TX_WAIT_FOR_FC;
                ctx_p->tx_link.base.active = true;
            }
        }
    }
    return status;
}

size_t ISOTP_Receive(struct isotp_ctx_t *ctx_p, void *destination_p, size_t length)
{
    assert(ctx_p != NULL);
    assert(destination_p != NULL);

    return Stream_Read(&ctx_p->rx_link.rx_stream, destination_p, length);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static inline void ConfigureRxLink(struct isotp_recv_link_t *link_p, void *rx_buffer_p, size_t rx_buffer_size, uint16_t rx_id, uint16_t tx_id, uint8_t separation_time, isotp_status_callback_t callback_fp)
{
    Stream_Init(&link_p->rx_stream, rx_buffer_p, rx_buffer_size);
    link_p->base.rx_id = rx_id;
    link_p->base.tx_id = tx_id;
    link_p->base.separation_time = separation_time;
    link_p->base.frame_fifo = FIFO_New(link_p->base.frame_buffer);
    link_p->base.callback_fp = callback_fp;
    link_p->base.active = true;
    link_p->state = ISOTP_RX_WAIT_FOR_FF_SF;

    const logging_logger_t *logger_p = Logging_GetLogger(ISOTP_LOGGER_NAME);
    Logging_Info(logger_p, "RX-link: {id: 0x%x, separation_time: %u, cb: 0x%x}", link_p->base.rx_id, link_p->base.separation_time, (uintptr_t)link_p->base.callback_fp);
}

static inline void ConfigureTxLink(struct isotp_send_link_t *link_p, void *tx_buffer_p, size_t tx_buffer_size, uint16_t rx_id, uint16_t tx_id, isotp_status_callback_t callback_fp)
{
    Stream_Init(&link_p->tx_stream, tx_buffer_p, tx_buffer_size);
    link_p->base.rx_id = rx_id;
    link_p->base.tx_id = tx_id;
    link_p->base.frame_fifo = FIFO_New(link_p->base.frame_buffer);
    link_p->base.callback_fp = callback_fp;
    link_p->base.active = false;
    link_p->state = ISOTP_TX_INACTIVE;

    const logging_logger_t *logger_p = Logging_GetLogger(ISOTP_LOGGER_NAME);
    Logging_Info(logger_p, "TX-link: {id: 0x%x, cb: 0x%x}", link_p->base.tx_id, (uintptr_t)link_p->base.callback_fp);
}

static void ProccessRxLink(struct isotp_recv_link_t *link_p)
{
    assert(link_p != NULL);

    switch (link_p->state)
    {
        case ISOTP_RX_WAIT_FOR_FF_SF:
            CheckForFirstAndSingleFrame(link_p);
            break;
        case ISOTP_RX_WAIT_FOR_CF:
            CheckForConsecutiveFrame(link_p);
            break;
        case ISOTP_RX_WAIT:
            CheckIfReadyForData(link_p);
            break;
        default:
            break;
    }
}

static void ProccessTxLink(struct isotp_send_link_t *link_p)
{
    assert(link_p != NULL);

    switch (link_p->state)
    {
        case ISOTP_TX_INACTIVE:
            break;
        case ISOTP_TX_SEND_CF:
            SendConsecutiveFrame(link_p);
            break;
        case ISOTP_TX_WAIT_FOR_ST:
            CheckIfSeparationTimeHasElapsed(link_p);
            break;
        case ISOTP_TX_WAIT_FOR_FC:
            CheckForFlowControlFrame(link_p);
            break;
        default:
            break;
    }
}

/* NOTE: This function may be called from an ISR. */
static void CanListener(const struct can_frame_t *frame_p, void *arg_p)
{
    assert(frame_p != NULL);
    assert(arg_p != NULL);

    /* Only use base link struct since this listener is shared by both RX and TX links. */
    struct isotp_link_t *link_p = (struct isotp_link_t *)arg_p;

    if (link_p->active && (frame_p->id == link_p->rx_id))
    {
        const bool status = FIFO_Push(&link_p->frame_fifo, frame_p);
        if (!status)
        {
            /**
             * The frame fifo should never be full due to the use of flow control. If it happens
             * it's probably an unrelated frame and can safely be discarded. Otherwise the missing
             * frame will be detected later on and the transfer will abort/timeout.
             */
            const logging_logger_t *logger_p = Logging_GetLogger(ISOTP_LOGGER_NAME);
            Logging_Warning(logger_p, "Discarded frame: {frame_id: %u, arg: 0x%x}", frame_p->id, (uintptr_t)arg_p);
        }
    }
}

static inline enum isotp_frame_type_t GetFrameType(const struct can_frame_t *frame_p)
{
    uint8_t code = (frame_p->data[0] & 0xF0) >> 4;
    enum isotp_frame_type_t result;
    if (code < ISOTP_FRAME_TYPE_END)
    {
        result = (enum isotp_frame_type_t)code;
    }
    else
    {
        result = ISOTP_FRAME_TYPE_END;
    }

    return result;
}

static void HandleSingleFrame(struct isotp_recv_link_t *link_p, const struct can_frame_t *frame_p)
{
    struct isotp_sf_t isotp_frame;
    memcpy(&isotp_frame, frame_p->data, frame_p->size);

    const logging_logger_t *logger_p = Logging_GetLogger(ISOTP_LOGGER_NAME);
    Logging_Debug(logger_p, "Received SF: {size: %u}", isotp_frame.size);

    if (isotp_frame.size <= Stream_GetAvailableSpace(&link_p->rx_stream))
    {
        /* No need to check the result, it's already verified that the data fits. */
        Stream_Write(&link_p->rx_stream, isotp_frame.data, isotp_frame.size);
        link_p->base.callback_fp(ISOTP_STATUS_DONE);
    }
    else
    {
        Stream_Clear(&link_p->rx_stream);
        link_p->base.callback_fp(ISOTP_STATUS_OVERFLOW_ABORT);
        Logging_Error(logger_p, "RX stream full");
    }
}

static void HandleFirstFrame(struct isotp_recv_link_t *link_p, const struct can_frame_t *frame_p)
{
    struct isotp_ff_t isotp_frame;
    memcpy(&isotp_frame, frame_p->data, frame_p->size);

    uint16_t total_size = (uint16_t)(isotp_frame.size_low | ((isotp_frame.size_high & 0x0F) << 8));
    const logging_logger_t *logger_p = Logging_GetLogger(ISOTP_LOGGER_NAME);
    Logging_Debug(logger_p, "Received FF: {total_size: %u}", total_size);

    /* Set rx_size early since it's needed when calling 'SendFlowControlFrame' */
    link_p->base.payload_size = total_size;
    if (sizeof(isotp_frame.data) <= Stream_GetAvailableSpace(&link_p->rx_stream))
    {
        /* No need to check the result is it's already checked that the data fits. */
        Stream_Write(&link_p->rx_stream, isotp_frame.data, sizeof(isotp_frame.data));

        link_p->base.sequence_number = 1;
        link_p->received_bytes = sizeof(isotp_frame.data);
        link_p->base.block_count = 0;
        link_p->state = ISOTP_RX_WAIT_FOR_CF;
        SendFlowControlFrame(link_p, ISOTP_FC_CONTINUE_TO_SEND);
    }
    else
    {
        SendFlowControlFrame(link_p, ISOTP_FC_OVERFLOW_ABORT);
        Stream_Clear(&link_p->rx_stream);
        link_p->base.callback_fp(ISOTP_STATUS_OVERFLOW_ABORT);
        Logging_Error(logger_p, "RX stream full");
    }
}

static void CheckForFirstAndSingleFrame(struct isotp_recv_link_t *link_p)
{
    struct can_frame_t frame;
    if (FIFO_Pop(&link_p->base.frame_fifo, &frame))
    {
        const enum isotp_frame_type_t frame_type = GetFrameType(&frame);
        switch (frame_type)
        {
            case ISOTP_SINGLE_FRAME:
                HandleSingleFrame(link_p, &frame);
                break;
            case ISOTP_FIRST_FRAME:
                HandleFirstFrame(link_p, &frame);
                break;
            default:
                break;
        }
    }
}

static void CheckForConsecutiveFrame(struct isotp_recv_link_t *link_p)
{
    struct can_frame_t frame;
    if (FIFO_Pop(&link_p->base.frame_fifo, &frame) && (GetFrameType(&frame) == ISOTP_CONSECUTIVE_FRAME))
    {
        HandleConsecutiveFrame(link_p, &frame);
    }
    else if (SysTime_GetDifference(link_p->wait_timer) > CF_TIMEOUT_MS)
    {
        link_p->state = ISOTP_RX_WAIT_FOR_FF_SF;
        Stream_Clear(&link_p->rx_stream);
        link_p->base.callback_fp(ISOTP_STATUS_TIMEOUT);
        const logging_logger_t *logger_p = Logging_GetLogger(ISOTP_LOGGER_NAME);
        Logging_Warning(logger_p, "Timeout while waiting for CF");
    }
    else
    {
        /* Do nothing and keep waiting for an consecutive frame. */
    }
}

static void HandleConsecutiveFrame(struct isotp_recv_link_t *link_p, const struct can_frame_t *frame_p)
{
    const logging_logger_t *logger_p = Logging_GetLogger(ISOTP_LOGGER_NAME);

    struct isotp_cf_t isotp_frame;
    memcpy(&isotp_frame, frame_p->data, frame_p->size);
    Logging_Debug(logger_p, "Received CF: {index: %u}", isotp_frame.index);

    if(isotp_frame.index == link_p->base.sequence_number)
    {
        size_t remaining_bytes = link_p->base.payload_size - link_p->received_bytes;
        if (remaining_bytes > sizeof(isotp_frame.data))
        {
            remaining_bytes = sizeof(isotp_frame.data);
        }

        if (remaining_bytes <= Stream_GetAvailableSpace(&link_p->rx_stream))
        {
            Stream_Write(&link_p->rx_stream, isotp_frame.data, remaining_bytes);

            link_p->received_bytes += remaining_bytes;
            link_p->base.sequence_number = (link_p->base.sequence_number + 1) % 16;
            link_p->base.block_count += 1;

            /* Change state if all data is received. */
            if (link_p->received_bytes == link_p->base.payload_size)
            {
                link_p->state = ISOTP_RX_WAIT_FOR_FF_SF;
                link_p->base.callback_fp(ISOTP_STATUS_DONE);
                Logging_Debug(logger_p, "Transfer complete");
            }
            else if (link_p->base.block_count == link_p->base.block_size)
            {
                SendFlowControlFrame(link_p, ISOTP_FC_CONTINUE_TO_SEND);
                link_p->base.block_count = 0;
            }
            else
            {
                Logging_Debug(logger_p, "Wait for CF: {received_bytes: %u}", link_p->received_bytes );
                /* Do nothing and wait for more consecutive frames. */
            }
        }
        else
        {
            SendFlowControlFrame(link_p, ISOTP_FC_OVERFLOW_ABORT);
            link_p->state = ISOTP_RX_WAIT_FOR_FF_SF;
            Logging_Error(logger_p, "RX stream full");
        }
    }
    else
    {
        /* Abort transfer if a frame is lost. */
        link_p->state = ISOTP_RX_WAIT_FOR_FF_SF;
        Stream_Clear(&link_p->rx_stream);
        link_p->base.callback_fp(ISOTP_STATUS_LOST_FRAME);
        Logging_Error(logger_p, "lost frame: {sn: %u, expected_sn: %u}", isotp_frame.index, link_p->base.sequence_number);
    }
}

static void SendFlowControlFrame(struct isotp_recv_link_t *link_p, enum isotp_flow_control_flag_t status)
{
    const logging_logger_t *logger_p = Logging_GetLogger(ISOTP_LOGGER_NAME);

    struct isotp_fc_t isotp_frame;
    isotp_frame.type = ISOTP_FLOW_CONTROL_FRAME;

    link_p->base.block_size = GetBlockSize(link_p);
    isotp_frame.block_size = link_p->base.block_size;

    if ((status == ISOTP_FC_CONTINUE_TO_SEND) && (isotp_frame.block_size == 0))
    {
        Logging_Warning(logger_p, "ISOTP_FC_WAIT");
        isotp_frame.flag = ISOTP_FC_WAIT;
        link_p->base.wf_count += 1;
        link_p->state = ISOTP_RX_WAIT;
        if (link_p->base.wf_count == 1)
        {
            link_p->base.callback_fp(ISOTP_STATUS_WAITING);
        }
    }
    else
    {
        isotp_frame.flag = (uint8_t)status;
        link_p->base.wf_count = 0;
    }

    link_p->wait_timer = SysTime_GetSystemTime();
    isotp_frame.st = (uint8_t)link_p->base.separation_time;

    Logging_Debug(logger_p, "Send FC {flag: %u, bs: %u, st: %u}", isotp_frame.flag, isotp_frame.block_size, isotp_frame.st);
    CANInterface_Transmit(link_p->base.tx_id, &isotp_frame, 3);
}

static void CheckIfReadyForData(struct isotp_recv_link_t *link_p)
{
    if (SysTime_GetDifference(link_p->wait_timer) > 100)
    {
        if (link_p->base.wf_count < WF_MAX)
        {
            link_p->state = ISOTP_RX_WAIT_FOR_CF;
            SendFlowControlFrame(link_p, ISOTP_FC_CONTINUE_TO_SEND);
        }
        else
        {
            const logging_logger_t *logger_p = Logging_GetLogger(ISOTP_LOGGER_NAME);
            Logging_Warning(logger_p, "Timeout while waiting for data/frame space");

            Stream_Clear(&link_p->rx_stream);
            SendFlowControlFrame(link_p, ISOTP_FC_OVERFLOW_ABORT);
            link_p->state = ISOTP_RX_WAIT_FOR_FF_SF;
            link_p->base.callback_fp(ISOTP_STATUS_TIMEOUT);
        }
    }
}

static uint8_t GetBlockSize(const struct isotp_recv_link_t *link_p)
{
    const size_t remaining_bytes = link_p->base.payload_size - link_p->received_bytes;
    const size_t stream_slot_size = remaining_bytes < CF_DATA_LENGTH ? remaining_bytes : CF_DATA_LENGTH;
    const size_t available_stream_slots = stream_slot_size > 0 ? (Stream_GetAvailableSpace(&link_p->rx_stream) / stream_slot_size) : 0;
    const uint8_t available_frame_slots = FIFO_GetAvailableSlots(&link_p->base.frame_fifo);

    uint8_t block_size;
    if (available_stream_slots > available_frame_slots)
    {
        block_size = available_frame_slots;
    }
    else
    {
        block_size = available_stream_slots < UINT8_MAX ? (uint8_t)available_stream_slots : UINT8_MAX;
    }
    return block_size;
}

static bool SendSingleFrame(const struct isotp_send_link_t *link_p, const void *data_p, size_t length)
{
    struct isotp_sf_t isotp_frame;
    isotp_frame.type = ISOTP_SINGLE_FRAME;
    isotp_frame.size = (uint8_t)length;
    memcpy(isotp_frame.data, data_p, length);

    const logging_logger_t *logger_p = Logging_GetLogger(ISOTP_LOGGER_NAME);
    Logging_Debug(logger_p, "Send SF: {total_size: %u}", length);

    const size_t frame_size = length + 1;
    return CANInterface_Transmit(link_p->base.tx_id, &isotp_frame, frame_size);
}

static bool SendFirstFrame(struct isotp_send_link_t *link_p, size_t length)
{
    struct isotp_ff_t isotp_frame;
    isotp_frame.type = ISOTP_FIRST_FRAME;
    isotp_frame.size_low = (uint8_t)(length & 0xFF);
    isotp_frame.size_high = (uint8_t)((length >> 8) & 0x0F);
    Stream_Read(&link_p->tx_stream, isotp_frame.data, sizeof(isotp_frame.data));

    const logging_logger_t *logger_p = Logging_GetLogger(ISOTP_LOGGER_NAME);
    Logging_Debug(logger_p, "Send FF: {total_size: %u}", length);

    bool status = false;
    if (CANInterface_Transmit(link_p->base.tx_id, &isotp_frame, sizeof(isotp_frame.data) + 2))
    {
        link_p->sent_bytes = sizeof(isotp_frame.data);
        link_p->base.payload_size = length;
        link_p->base.sequence_number = 1;
        link_p->wait_timer = SysTime_GetSystemTime();
        link_p->state = ISOTP_TX_WAIT_FOR_FC;
        status = true;
    }

    return status;
}

static void CheckIfSeparationTimeHasElapsed(struct isotp_send_link_t *link_p)
{
    const uint32_t time_diff_us = SysTime_GetSystemTimeUs() - link_p->wait_timer;
    if (time_diff_us >= link_p->base.separation_time)
    {
        link_p->state = ISOTP_TX_SEND_CF;
    }
}

static void CheckForFlowControlFrame(struct isotp_send_link_t *link_p)
{
    struct can_frame_t frame;
    if (FIFO_Pop(&link_p->base.frame_fifo, &frame) && (GetFrameType(&frame) == ISOTP_FLOW_CONTROL_FRAME))
    {
        HandleFlowControlFrame(link_p, &frame);
    }
    else if (SysTime_GetDifference(link_p->wait_timer) > FC_TIMEOUT_MS)
    {
        link_p->state = ISOTP_TX_INACTIVE;
        link_p->base.active = false;
        link_p->base.callback_fp(ISOTP_STATUS_TIMEOUT);
        const logging_logger_t *logger_p = Logging_GetLogger(ISOTP_LOGGER_NAME);
        Logging_Warning(logger_p, "Timeout while waiting for FC");
    }
    else
    {
        /* Do nothing and keep waiting for a flow control frame. */
    }
}

static void HandleFlowControlFrame(struct isotp_send_link_t *link_p, const struct can_frame_t *frame_p)
{
    const logging_logger_t *logger_p = Logging_GetLogger(ISOTP_LOGGER_NAME);

    struct isotp_fc_t isotp_frame;
    memcpy(&isotp_frame, frame_p->data, frame_p->size);
    Logging_Debug(logger_p, "Received FC {flag: %u, bs: %u, st: %u}", isotp_frame.flag, isotp_frame.block_size, isotp_frame.st);

    switch (isotp_frame.flag)
    {
        case ISOTP_FC_CONTINUE_TO_SEND:
            link_p->base.block_size = isotp_frame.block_size;
            link_p->base.block_count = 0;
            link_p->base.separation_time = SeparationTimeToUs(isotp_frame.st);
            link_p->state = ISOTP_TX_SEND_CF;
            break;
        case ISOTP_FC_WAIT:
            if (link_p->base.wf_count >= WF_MAX)
            {
                /* Abort if max number of WAIT is exceeded. */
                link_p->base.callback_fp(ISOTP_STATUS_TIMEOUT);
                link_p->state = ISOTP_TX_INACTIVE;
                link_p->base.active = false;
                Logging_Warning(logger_p, "Max number of wait indications exceeded: {wf_count: %u}", link_p->base.wf_count);
            }
            else
            {
                /* Keep waiting for another flow control frame. */
                link_p->state = ISOTP_TX_WAIT_FOR_FC;
                link_p->base.wf_count += 1;
            }
            break;
        case ISOTP_FC_OVERFLOW_ABORT:
            /* Abort at client request. */
            link_p->base.callback_fp(ISOTP_STATUS_OVERFLOW_ABORT);
            link_p->state = ISOTP_TX_INACTIVE;
            link_p->base.active = false;
            Logging_Warning(logger_p, "Transfer aborted by receiver");
            break;
        default:
            Logging_Error(logger_p, "Invalid status flag: {flag: %u}", isotp_frame.flag);
            break;
    }
}

static uint32_t SeparationTimeToUs(uint8_t st)
{
    uint32_t result = 10000;
    if (st <= 127)
    {
        result = st * 1000;
    }
    else if ((st >= 0xF1) && (st <= 0xF9))
    {
        result = (st - 0xF0) * 100;
    }
    else
    {
        /* Use default value if separation time is invalid. */
        const logging_logger_t *logger_p = Logging_GetLogger(ISOTP_LOGGER_NAME);
        Logging_Warning(logger_p, "Invalid st: {st: %u}", st);
    }

    return result;
}

static bool SendConsecutiveFrame(struct isotp_send_link_t *link_p)
{
    struct isotp_cf_t frame;
    frame.type = ISOTP_CONSECUTIVE_FRAME;
    frame.index = link_p->base.sequence_number;
    const size_t number_of_bytes = Stream_Read(&link_p->tx_stream, frame.data, sizeof(frame.data));

    const logging_logger_t *logger_p = Logging_GetLogger(ISOTP_LOGGER_NAME);
    Logging_Debug(logger_p, "Send CF: {index: %u, number_of_bytes: %u}", frame.index, number_of_bytes);

    bool status = false;
    if (CANInterface_Transmit(link_p->base.tx_id, &frame, number_of_bytes + 1))
    {
        link_p->sent_bytes += number_of_bytes;

        if (link_p->sent_bytes < link_p->base.payload_size)
        {
            link_p->base.sequence_number += 1;
            link_p->base.block_count += 1;

            if ((link_p->base.block_count < link_p->base.block_size) || (link_p->base.block_size == 0))
            {
                if (link_p->base.separation_time == 0)
                {
                    link_p->state = ISOTP_TX_SEND_CF;
                }
                else
                {
                    link_p->wait_timer = SysTime_GetSystemTimeUs();
                    link_p->state = ISOTP_TX_WAIT_FOR_ST;
                }
            }
            else
            {
                link_p->wait_timer = SysTime_GetSystemTime();
                link_p->state = ISOTP_TX_WAIT_FOR_FC;
            }
        }
        else
        {
            link_p->base.callback_fp(ISOTP_STATUS_DONE);
            link_p->state = ISOTP_TX_INACTIVE;
            link_p->base.active = false;
        }
        status = true;
    }
    else
    {
        link_p->base.callback_fp(ISOTP_STATUS_OVERFLOW_ABORT);
        link_p->state = ISOTP_TX_INACTIVE;
        link_p->base.active = false;
    }

    return status;
}
