/**
 * @file   adc.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  ADC module.
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

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/dma.h>

#include <assert.h>
#include "utility.h"
#include "logging.h"
#include "adc.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define ADC_LOGGER_NAME "ADC"
#ifndef ADC_LOGGER_DEBUG_LEVEL
#define ADC_LOGGER_DEBUG_LEVEL LOGGING_DEBUG
#endif

#define NUMBER_OF_READINGS_PER_SAMPLE 16
#define MAX_NUMBER_OF_CHANNELS 2

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct module_t
{
    logging_logger_t *logger;
    volatile uint32_t sample_buffer[NUMBER_OF_READINGS_PER_SAMPLE * MAX_NUMBER_OF_CHANNELS];
    size_t number_of_channels;
    adc_input_t *channels[MAX_NUMBER_OF_CHANNELS];
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct module_t module;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static void SetupNVIC(void);
static void SetupDMA(void);
static void SetupADC(void);
static inline uint32_t SampleToVoltage(uint32_t sample);
static inline void UpdateSamples(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void ADC_Init(void)
{
    module = (__typeof__(module)) {0};
    module.logger = Logging_GetLogger(ADC_LOGGER_NAME);
    Logging_SetLevel(module.logger, ADC_LOGGER_DEBUG_LEVEL);

    SetupNVIC();
    SetupDMA();
    SetupADC();

    Logging_Info(module.logger, "ADC initialized");
}

void ADC_InitChannel(adc_input_t *self_p, uint8_t channel)
{
    assert(self_p != NULL);
    assert(module.number_of_channels < ElementsIn(module.channels));

    *self_p = (__typeof__(*self_p)) {0};
    self_p->channel = channel;

    module.channels[module.number_of_channels] = self_p;
    ++module.number_of_channels;

    Logging_Info(module.logger, "Initialized ADC channel %u", channel);
}

void ADC_Start(void)
{
    uint8_t channels[ElementsIn(module.channels)];
    for (size_t i = 0; i < module.number_of_channels; ++i)
    {
        channels[i] = module.channels[i]->channel;
    }

    adc_set_regular_sequence(ADC1, (uint8_t)module.number_of_channels, channels);
    adc_start_conversion_regular(ADC1);

    Logging_Info(module.logger, "Scanning on %u channel(s).", module.number_of_channels);
}

uint32_t ADC_GetVoltage(const adc_input_t *self_p)
{
    assert(self_p != NULL);

    return SampleToVoltage(self_p->value);
}

#ifdef UNIT_TEST
volatile uint32_t *ADC_GetSampleBuffer(void)
{
    return module.sample_buffer;
}
#endif

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static inline void Nop(void)
{
    __asm__("nop");
}

static void Delay(void)
{
    for (size_t i = 0; i < 800000; ++i)
    {
        Nop();
    }
}

static void SetupNVIC(void)
{
    nvic_set_priority(NVIC_DMA1_CHANNEL1_IRQ, 1);
    nvic_enable_irq(NVIC_DMA1_CHANNEL1_IRQ);
}

static void SetupDMA(void)
{
    const uint32_t dma = DMA1;
    const uint8_t channel = DMA_CHANNEL1;

    rcc_periph_clock_enable(RCC_DMA1);

    dma_channel_reset(dma, channel);
    dma_enable_circular_mode(dma, channel);
    dma_enable_memory_increment_mode(dma, channel);
    dma_set_memory_size(dma, channel, DMA_CCR_MSIZE_32BIT);
    dma_set_memory_address(dma, channel, (uintptr_t)module.sample_buffer);
    dma_set_number_of_data(dma, channel, ElementsIn(module.sample_buffer));
    dma_set_read_from_peripheral(dma, channel);
    dma_set_peripheral_address(dma, channel, (uintptr_t)&ADC_DR(ADC1));
    dma_set_peripheral_size(dma, channel, DMA_CCR_PSIZE_32BIT);
    dma_set_priority(dma, channel, DMA_CCR_PL_HIGH);
    dma_enable_transfer_complete_interrupt(dma, channel);
    dma_enable_channel(dma, channel);
}

static void SetupADC(void)
{
    rcc_periph_clock_enable(RCC_ADC1);
    adc_power_off(ADC1);
    rcc_periph_reset_pulse(RST_ADC1);

    adc_enable_scan_mode(ADC1);
    adc_set_continuous_conversion_mode(ADC1);
    adc_enable_dma(ADC1);
    adc_enable_external_trigger_regular(ADC1, ADC_CR2_EXTSEL_SWSTART);
    adc_set_right_aligned(ADC1);
    adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_28DOT5CYC);
    adc_power_on(ADC1);

    Logging_Debug(module.logger, "Calibrate ADC...");
    Delay();
    adc_reset_calibration(ADC1);
    adc_calibrate(ADC1);
}

static inline uint32_t SampleToVoltage(uint32_t sample)
{
    const uint32_t reference_voltage = 3300;
    const uint32_t adc_resolution = 4096;

    return (sample * reference_voltage) / adc_resolution;
}

static inline void UpdateSamples(void)
{
    for (size_t channel_index = 0; channel_index < module.number_of_channels; ++channel_index)
    {
        const size_t total_number_of_readings = module.number_of_channels * NUMBER_OF_READINGS_PER_SAMPLE;
        uint32_t sample_sum = 0;

        for (size_t i = channel_index; i < total_number_of_readings; i += module.number_of_channels)
        {
            sample_sum += module.sample_buffer[i] & 0xFF00;
        }

        module.channels[channel_index]->value = sample_sum / NUMBER_OF_READINGS_PER_SAMPLE;
    }
}

void dma1_channel1_isr(void)
{
    UpdateSamples();

    const uint32_t dma = DMA1;
    const uint8_t channel = DMA_CHANNEL1;
    dma_clear_interrupt_flags(dma, channel, DMA_TCIF);
}
