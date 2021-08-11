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
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

void dma1_channel1_isr(void);

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

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

static void ExpectADCConversionStart(uint8_t *channels_p, size_t number_of_channels)
{
    const uint32_t adc = ADC1;

    expect_value(adc_set_regular_sequence, adc, adc);
    expect_value(adc_set_regular_sequence, length, number_of_channels);
    expect_value(adc_set_regular_sequence, channel, channels_p);
    expect_value(adc_start_conversion_regular, adc, adc);
}

static void ClearSampleBuffer()
{
    const size_t number_of_readings_per_sample = 16;
    uint32_t *sample_buffer_p = ADC_GetSampleBuffer();

    for (size_t i = 0; i < number_of_readings_per_sample; ++i)
    {
        sample_buffer_p[i] = 0;
    }
}

static void FillSampleBuffer(uint32_t value, size_t number_of_samples, uint8_t offset, uint8_t step)
{
    const size_t number_of_readings_per_sample = 16;
    uint32_t *sample_buffer_p = ADC_GetSampleBuffer();

    for (size_t i = offset; i < number_of_samples * step; i += step)
    {
        sample_buffer_p[i] = value;
    }
}

static int Setup(void **state)
{
    will_return(Logging_GetLogger, dummy_logger);
    ExpectDMASetup();
    ExpectADCSetup();
    ADC_Init();
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_ADC_Init(void **state)
{
    will_return(Logging_GetLogger, dummy_logger);
    ExpectDMASetup();
    ExpectADCSetup();
    ADC_Init();
}

static void test_ADC_Start_Invalid(void **state)
{
    uint8_t channels[3];

    expect_assert_failure(ADC_Start(NULL, 0));
    expect_assert_failure(ADC_Start(channels, ElementsIn(channels)));
}

static void test_ADC_Start(void **state)
{
    uint8_t channels[2] = {0, 1};

    ExpectADCConversionStart(channels, ElementsIn(channels));
    ADC_Start(channels, ElementsIn(channels));
}

static void test_ADC_GetVoltage_Invalid(void **state)
{
    expect_assert_failure(ADC_GetVoltage(3));
}

static void test_ADC_GetVoltage(void **state)
{
    const size_t number_of_readings_per_sample = 16;
    uint8_t channels[2] = {2, 3};

    /* One channel */
    uint8_t number_of_channels = 1;
    ExpectADCConversionStart(channels, number_of_channels);
    ADC_Start(channels, number_of_channels);

    ClearSampleBuffer();
    FillSampleBuffer(4096, number_of_readings_per_sample, 0, number_of_channels);
    dma1_channel1_isr();
    assert_int_equal(ADC_GetVoltage(0), 3300);

    ClearSampleBuffer();
    FillSampleBuffer(2048, number_of_readings_per_sample, 0, number_of_channels);
    dma1_channel1_isr();
    assert_int_equal(ADC_GetVoltage(0), 1650);

    ClearSampleBuffer();
    FillSampleBuffer(4096, number_of_readings_per_sample / 2, 0, number_of_channels);
    dma1_channel1_isr();
    assert_int_equal(ADC_GetVoltage(0), 1650);

    /* Two channels */
    number_of_channels = 2;
    ExpectADCConversionStart(channels, number_of_channels);
    ADC_Start(channels, number_of_channels);

    ClearSampleBuffer();
    FillSampleBuffer(2048, number_of_readings_per_sample, 0, number_of_channels);
    FillSampleBuffer(4096, number_of_readings_per_sample, 1, number_of_channels);
    dma1_channel1_isr();
    assert_int_equal(ADC_GetVoltage(0), 1650);
    assert_int_equal(ADC_GetVoltage(1), 3300);
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_ADC[] =
    {
        cmocka_unit_test(test_ADC_Init),
        cmocka_unit_test_setup(test_ADC_Start_Invalid, Setup),
        cmocka_unit_test_setup(test_ADC_Start, Setup),
        cmocka_unit_test_setup(test_ADC_GetVoltage_Invalid, Setup),
        cmocka_unit_test_setup(test_ADC_GetVoltage, Setup),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_ADC, NULL, NULL);
}
