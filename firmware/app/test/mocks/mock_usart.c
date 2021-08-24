/**
 * @file   mock_usart.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for usart.
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
#include <stdbool.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <libopencm3/stm32/usart.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

__attribute__((weak)) void usart_set_baudrate(uint32_t usart, uint32_t baud)
{
    check_expected(usart);
    check_expected(baud);
}

__attribute__((weak)) void usart_set_databits(uint32_t usart, uint32_t bits)
{

}

__attribute__((weak)) void usart_set_stopbits(uint32_t usart, uint32_t stopbits)
{

}

__attribute__((weak)) void usart_set_parity(uint32_t usart, uint32_t parity)
{

}

__attribute__((weak)) void usart_set_mode(uint32_t usart, uint32_t mode)
{

}

__attribute__((weak)) void usart_set_flow_control(uint32_t usart, uint32_t flowcontrol)
{

}

__attribute__((weak)) void usart_enable(uint32_t usart)
{
    check_expected(usart);
}

__attribute__((weak)) void usart_disable(uint32_t usart)
{

}

__attribute__((weak)) void usart_send(uint32_t usart, uint16_t data)
{

}

__attribute__((weak)) uint16_t usart_recv(uint32_t usart)
{
    return mock_type(uint16_t);
}

__attribute__((weak)) void usart_wait_send_ready(uint32_t usart)
{

}

__attribute__((weak)) void usart_wait_recv_ready(uint32_t usart)
{

}

__attribute__((weak)) void usart_send_blocking(uint32_t usart, uint16_t data)
{
    check_expected(usart);
    check_expected(data);
}

__attribute__((weak)) uint16_t usart_recv_blocking(uint32_t usart)
{

}

__attribute__((weak)) void usart_enable_rx_dma(uint32_t usart)
{

}

__attribute__((weak)) void usart_disable_rx_dma(uint32_t usart)
{

}

__attribute__((weak)) void usart_enable_tx_dma(uint32_t usart)
{

}

__attribute__((weak)) void usart_disable_tx_dma(uint32_t usart)
{

}

__attribute__((weak)) void usart_enable_rx_interrupt(uint32_t usart)
{

}

__attribute__((weak)) void usart_disable_rx_interrupt(uint32_t usart)
{

}

__attribute__((weak)) void usart_enable_tx_interrupt(uint32_t usart)
{

}

__attribute__((weak)) void usart_disable_tx_interrupt(uint32_t usart)
{

}

__attribute__((weak)) void usart_enable_tx_complete_interrupt(uint32_t usart)
{

}

__attribute__((weak)) void usart_disable_tx_complete_interrupt(uint32_t usart)
{

}

__attribute__((weak)) void usart_enable_idle_interrupt(uint32_t usart)
{

}

__attribute__((weak)) void usart_disable_idle_interrupt(uint32_t usart)
{

}

__attribute__((weak)) void usart_enable_error_interrupt(uint32_t usart)
{

}

__attribute__((weak)) void usart_disable_error_interrupt(uint32_t usart)
{

}

__attribute__((weak)) bool usart_get_flag(uint32_t usart, uint32_t flag)
{
    return mock_type(bool);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
