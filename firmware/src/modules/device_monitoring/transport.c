/**
 * @file   transport.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
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
#include "utility.h"
#include "isotp.h"
#include "transport.h"

#include "memfault/core/data_packetizer.h"

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

void Transport_Init(void)
{
    module = (__typeof__(module)) {0};

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

    bool data_available = memfault_packetizer_get_chunk(data, &number_of_bytes);
    if (data_available )
    {
        bool status = ISOTP_Send(&module.ctx, data, number_of_bytes);
    }

    ISOTP_Proccess(&module.ctx);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static void RxStatusCallback(enum isotp_status_t status)
{
    switch(status)
    {
        case ISOTP_STATUS_DONE:
        case ISOTP_STATUS_WAITING:
        case ISOTP_STATUS_TIMEOUT:
        case ISOTP_STATUS_LOST_FRAME:
        case ISOTP_STATUS_OVERFLOW_ABORT:
        default:
            break;
    }
}

static void TxStatusCallback(enum isotp_status_t status)
{
    switch(status)
    {
        case ISOTP_STATUS_DONE:
        case ISOTP_STATUS_WAITING:
        case ISOTP_STATUS_TIMEOUT:
        case ISOTP_STATUS_LOST_FRAME:
        case ISOTP_STATUS_OVERFLOW_ABORT:
        default:
            break;
    }
}
