/**
 * @file   isotp.h
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

#ifndef ISOTP_H_
#define ISOTP_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "fifo.h"
#include "stream.h"
#include "logging.h"
#include "can_interface.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define ISOTP_FRAME_BUFFER_SIZE 5

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

enum isotp_status_t
{
    ISOTP_STATUS_DONE = 0,
    ISOTP_STATUS_WAITING,
    ISOTP_STATUS_TIMEOUT,
    ISOTP_STATUS_LOST_FRAME,
    ISOTP_STATUS_OVERFLOW_ABORT
};

typedef void (*isotp_status_callback_t)(enum isotp_status_t status);

enum isotp_tx_state_t
{
    ISOTP_TX_INACTIVE = 0,
    ISOTP_TX_SEND_CF,
    ISOTP_TX_WAIT_FOR_FC
};

enum isotp_rx_state_t
{
    ISOTP_RX_WAIT_FOR_FF_SF = 0,
    ISOTP_RX_WAIT_FOR_CF,
    ISOTP_RX_WAIT
};

enum isotp_link_type_t
{
    ISOTP_LINK_RX = 0,
    ISOTP_LINK_TX
};

struct isotp_link_t
{
    uint16_t rx_id;
    uint16_t tx_id;
    uint8_t separation_time;
    uint8_t block_size;
    uint8_t block_count;
    uint8_t sequence_number;
    size_t payload_size;
    uint8_t wf_count;
    isotp_status_callback_t callback_fp;
    struct can_frame_t frame_buffer[ISOTP_FRAME_BUFFER_SIZE];
    struct fifo_t frame_fifo;
    bool active;
};

struct isotp_recv_link_t
{
    struct isotp_link_t base;
    size_t received_bytes;
    uint32_t wait_timer;
    struct stream_t rx_stream;
    enum isotp_rx_state_t state;
};

struct isotp_send_link_t
{
    struct isotp_link_t base;
    size_t sent_bytes;
    uint32_t wait_timer;
    const uint8_t *data_p;
    enum isotp_tx_state_t state;
};

struct isotp_ctx_t
{
    struct isotp_recv_link_t rx_link;
    struct isotp_send_link_t tx_link;
    logging_logger_t *logger_p;
};

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

void ISOTP_Bind(struct isotp_ctx_t *ctx_p, void *rx_buffer_p, size_t rx_buffer_size, uint16_t rx_id, uint16_t tx_id, uint8_t separation_time, isotp_status_callback_t rx_callback_fp, isotp_status_callback_t tx_callback_fp);
void ISOTP_Proccess(struct isotp_ctx_t *ctx_p);
size_t ISOTP_Receive(struct isotp_ctx_t *ctx_p, void *destination_p, size_t length);
bool ISOTP_Send(struct isotp_ctx_t *ctx_p, const void *data_p, size_t length);
void ISOTP_ProccessTx(struct isotp_ctx_t *ctx_p);

#endif
