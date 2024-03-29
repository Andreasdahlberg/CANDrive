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

enum isotp_tx_state_t
{
    ISOTP_TX_INACTIVE = 0,
    ISOTP_TX_SEND_CF,
    ISOTP_TX_WAIT_FOR_ST,
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

typedef void (*isotp_status_callback_t)(enum isotp_status_t status);

struct isotp_link_t
{
    uint16_t rx_id;
    uint16_t tx_id;
    uint32_t separation_time;
    uint8_t block_size;
    uint8_t block_count;
    uint8_t sequence_number;
    uint8_t wf_count;
    size_t payload_size;
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
    struct stream_t tx_stream;
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
/**
 * Create a link between two endpoints.
 *
 * @param ctx_p Pointer to link context.
 * @param rx_buffer_p Pointer to RX buffer.
 * @param rx_buffer_size Size of RX buffer.
 * @param tx_buffer_p Pointer to TX buffer.
 * @param tx_buffer_size Size of TX buffer.
 * @param rx_id ID of RX endpoint.
 * @param tx_id ID of TX endpoint
 * @param rx_callback_fp Callback for RX events.
 * @param tx_callback_fp Callback for TX events.
 */
void ISOTP_Bind(struct isotp_ctx_t *ctx_p, void *rx_buffer_p, size_t rx_buffer_size, void *tx_buffer_p, size_t tx_buffer_size, uint16_t rx_id, uint16_t tx_id, isotp_status_callback_t rx_callback_fp, isotp_status_callback_t tx_callback_fp);

/**
 * Set separation time parameter.
 *
 * @param ctx_p Pointer to link context.
 * @param separation_time Separation time between frames.
 */
void ISOTP_SetSeparationTime(struct isotp_ctx_t *ctx_p, uint8_t separation_time);

/**
 * Process RX and TX data.
 *
 * @param ctx_p Pointer to link context.
 */
void ISOTP_Proccess(struct isotp_ctx_t *ctx_p);

/**
 * Send data.
 *
 * @param ctx_p Pointer to link context.
 * @param data_p Pointer to data.
 * @param length Number of bytes to send.
 *
 * @return True if send was successful, otherwise false.
 */
bool ISOTP_Send(struct isotp_ctx_t *ctx_p, const void *data_p, size_t length);

/**
 * Check if the link is currently sending.
 *
 * @param ctx_p Pointer to link context.
 * @return True if sending, otherwise false.
 */
bool ISOTP_IsSending(const struct isotp_ctx_t *ctx_p);

/**
 * Receive any available data.
 *
 * @param ctx_p Pointer to link context.
 * @param destination_p Pointer to destination buffer where data will be stored.
 * @param length Max number of bytes to receive(size of destination buffer).
 *
 * @return Number of bytes received, zero if no data is available.
 */
size_t ISOTP_Receive(struct isotp_ctx_t *ctx_p, void *destination_p, size_t length);

#endif
