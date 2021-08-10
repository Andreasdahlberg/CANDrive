/**
 * @file   test_adc.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the ADC module.
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
#include "utility.h"
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/adc.h>
#include "adc.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

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

    return 0;
}

static void ExpectDMASetup(void)
{
    const uint32_t dma = DMA1;
    const uint8_t channel = DMA_CHANNEL1;

    expect_value(dma_channel_reset, dma, dma);
    expect_value(dma_channel_reset, channel, channel);
    expect_value(dma_enable_circular_mode, dma, dma);
    expect_value(dma_enable_circular_mode, channel, channel);
    expect_value(dma_enable_memory_increment_mode, dma, dma);
    expect_value(dma_enable_memory_increment_mode, channel, channel);
    expect_value(dma_set_read_from_peripheral, dma, dma);
    expect_value(dma_set_read_from_peripheral, channel, channel);
    expect_value(dma_enable_transfer_complete_interrupt, dma, dma);
    expect_value(dma_enable_transfer_complete_interrupt, channel, channel);
    expect_value(dma_enable_channel, dma, dma);
    expect_value(dma_enable_channel, channel, channel);
    expect_value(dma_set_memory_address, dma, dma);
    expect_value(dma_set_memory_address, channel, channel);

    const uint16_t number_of_data = 32;
    expect_value(dma_set_number_of_data, dma, dma);
    expect_value(dma_set_number_of_data, channel, channel);
    expect_value(dma_set_number_of_data, number, number_of_data);
}

static void ExpectADCSetup(void)
{
    const uint32_t adc = ADC1;

    expect_value(adc_enable_scan_mode, adc, adc);
    expect_value(adc_enable_dma, adc, adc);
    expect_value(adc_power_on, adc, adc);
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

void test_ADC_Init(void **state)
{
    will_return(Logging_GetLogger, dummy_logger);
    ExpectDMASetup();
    ExpectADCSetup();
    ADC_Init();
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_ADC[] =
    {
        cmocka_unit_test(test_ADC_Init),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_ADC, NULL, NULL);
}
