/**
 * @file   transport.c
 * @author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Transport link for debug data.
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
#include <stddef.h>
#include <string.h>
#include "memfault/core/data_packetizer.h"
#include "utility.h"
#include "isotp.h"
#include "transport.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define RX_BUFFER_SIZE 32
#define TX_BUFFER_SIZE 256

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct module_t
{
    logging_logger_t *logger_p;
    struct isotp_ctx_t ctx;
    uint8_t rx_buffer[RX_BUFFER_SIZE];
    uint8_t tx_buffer[TX_BUFFER_SIZE];
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct module_t module;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static void RxStatusCallback(enum isotp_status_t status);
static void TxStatusCallback(enum isotp_status_t status);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void Transport_Init(logging_logger_t *logger_p)
{
    module = (__typeof__(module)) {0};
    module.logger_p = logger_p;

    ISOTP_Bind(&module.ctx,
               module.rx_buffer,
               sizeof(module.rx_buffer),
               module.tx_buffer,
               sizeof(module.tx_buffer),
               0x03,
               0x04,
               RxStatusCallback,
               TxStatusCallback
              );
}

void Transport_Update(void)
{
    uint8_t data[64];
    size_t number_of_bytes = sizeof(data);

    if (!ISOTP_IsSending(&module.ctx) && memfault_packetizer_get_chunk(data, &number_of_bytes))
    {
        Logging_Debug(module.logger_p, "Chunk available: {length: %u}", number_of_bytes);
        if (!ISOTP_Send(&module.ctx, data, number_of_bytes))
        {
            Logging_Warning(module.logger_p, "Failed to send chunk: {length: %u}", number_of_bytes);
            memfault_packetizer_abort();
        }
    }

    ISOTP_Proccess(&module.ctx);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static void RxStatusCallback(enum isotp_status_t status __attribute__((unused)))
{
    /* Empty callback. */
}

static void TxStatusCallback(enum isotp_status_t status)
{
    switch(status)
    {
        case ISOTP_STATUS_DONE:
            break;
        case ISOTP_STATUS_WAITING:
            /**
             * Do nothing here, wait for receiver. No need to check for timeout
             * since it's handled by the ISO-TP layer.
             */
            break;
        case ISOTP_STATUS_TIMEOUT:
        case ISOTP_STATUS_LOST_FRAME:
        case ISOTP_STATUS_OVERFLOW_ABORT:
            Logging_Warning(module.logger_p, "Abort due to ISOTP error: {status: %u}", (uint32_t)status);
            memfault_packetizer_abort();
            break;
        default:
            Logging_Error(module.logger_p, "Unknown status: {status: %u}", (uint32_t)status);
            memfault_packetizer_abort();
            break;
    }
}
