/**
 * @file   test_serial.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the Serial module.
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
#include <libopencm3/stm32/usart.h>
#include "utility.h"
#include "serial.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define TEST_USART USART2

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static int Setup(void **state)
{
    const uint32_t baud_rate = 921600;

    expect_value(usart_set_baudrate, usart, TEST_USART);
    expect_value(usart_set_baudrate, baud, baud_rate);
    expect_value(usart_enable, usart, TEST_USART);
    Serial_Init(baud_rate);

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

void test_Serial_Init(void **state)
{
    const uint32_t baud_rates[] = {115200, 921600};

    for (size_t i = 0; i < ElementsIn(baud_rates); ++i)
    {
        const uint32_t baud_rate = baud_rates[i];

        expect_value(usart_set_baudrate, usart, TEST_USART);
        expect_value(usart_set_baudrate, baud, baud_rate);
        expect_value(usart_enable, usart, TEST_USART);
        Serial_Init(baud_rate);
    }
}

void test_Serial_Send(void **state)
{
    const char data[] = "abcdefgh";

    /* Expect nothing since no data is sent. */
    Serial_Send(data, 0);

    for (size_t i = 0; i < strlen(data); ++i)
    {
        expect_value(usart_send_blocking, usart, TEST_USART);
        expect_value(usart_send_blocking, data, data[i]);
    }

    Serial_Send(data, strlen(data));
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_FIFO[] =
    {
        cmocka_unit_test(test_Serial_Init),
        cmocka_unit_test_setup(test_Serial_Send, Setup)
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_FIFO, NULL, NULL);
}
