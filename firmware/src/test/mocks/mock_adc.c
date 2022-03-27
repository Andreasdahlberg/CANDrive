/**
 * @file   mock_adc.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for adc.
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
#include <libopencm3/stm32/adc.h>

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

__attribute__((weak)) void adc_power_on(uint32_t adc)
{
    check_expected(adc);
}

__attribute__((weak)) void adc_power_off(uint32_t adc)
{
}

__attribute__((weak)) void adc_enable_analog_watchdog_regular(uint32_t adc)
{
}

__attribute__((weak)) void adc_disable_analog_watchdog_regular(uint32_t adc)
{
}

__attribute__((weak)) void adc_enable_analog_watchdog_injected(uint32_t adc)
{
}

__attribute__((weak)) void adc_disable_analog_watchdog_injected(uint32_t adc)
{
}

__attribute__((weak)) void adc_enable_discontinuous_mode_regular(uint32_t adc, uint8_t length)
{
}

__attribute__((weak)) void adc_disable_discontinuous_mode_regular(uint32_t adc)
{
}

__attribute__((weak)) void adc_enable_discontinuous_mode_injected(uint32_t adc)
{
}

__attribute__((weak)) void adc_disable_discontinuous_mode_injected(uint32_t adc)
{
}

__attribute__((weak)) void adc_enable_automatic_injected_group_conversion(uint32_t adc)
{
}

__attribute__((weak)) void adc_disable_automatic_injected_group_conversion(uint32_t adc)
{
}

__attribute__((weak)) void adc_enable_analog_watchdog_on_all_channels(uint32_t adc)
{
}

__attribute__((weak)) void adc_enable_analog_watchdog_on_selected_channel(uint32_t adc, uint8_t channel)
{
}

__attribute__((weak)) void adc_enable_scan_mode(uint32_t adc)
{
    check_expected(adc);
}

__attribute__((weak)) void adc_disable_scan_mode(uint32_t adc)
{
}

__attribute__((weak)) void adc_enable_eoc_interrupt_injected(uint32_t adc)
{
}

__attribute__((weak)) void adc_disable_eoc_interrupt_injected(uint32_t adc)
{
}

__attribute__((weak)) void adc_enable_awd_interrupt(uint32_t adc)
{
}

__attribute__((weak)) void adc_disable_awd_interrupt(uint32_t adc)
{
}

__attribute__((weak)) void adc_enable_eoc_interrupt(uint32_t adc)
{
}

__attribute__((weak)) void adc_disable_eoc_interrupt(uint32_t adc)
{
}

__attribute__((weak)) void adc_set_left_aligned(uint32_t adc)
{
}

__attribute__((weak)) void adc_set_right_aligned(uint32_t adc)
{
}

__attribute__((weak)) bool adc_eoc(uint32_t adc)
{
    mock_type(bool);
}

__attribute__((weak)) bool adc_eoc_injected(uint32_t adc)
{
    mock_type(bool);
}

__attribute__((weak)) uint32_t adc_read_regular(uint32_t adc)
{
    mock_type(uint32_t);
}

__attribute__((weak)) uint32_t adc_read_injected(uint32_t adc, uint8_t reg)
{
    mock_type(uint32_t);
}

__attribute__((weak)) void adc_set_continuous_conversion_mode(uint32_t adc)
{
}

__attribute__((weak)) void adc_set_single_conversion_mode(uint32_t adc)
{
}

__attribute__((weak)) void adc_set_regular_sequence(uint32_t adc, uint8_t length, uint8_t channel[])
{
    check_expected(adc);
    check_expected(length);
    check_expected(channel);
}

__attribute__((weak)) void adc_set_injected_sequence(uint32_t adc, uint8_t length, uint8_t channel[])
{
}

__attribute__((weak)) void adc_set_injected_offset(uint32_t adc, uint8_t reg, uint32_t offset)
{
}

__attribute__((weak)) void adc_set_watchdog_high_threshold(uint32_t adc, uint16_t threshold)
{
}

__attribute__((weak)) void adc_set_watchdog_low_threshold(uint32_t adc, uint16_t threshold)
{
}

__attribute__((weak)) void adc_start_conversion_regular(uint32_t adc)
{
    check_expected(adc);
}

__attribute__((weak)) void adc_start_conversion_injected(uint32_t adc)
{
}

__attribute__((weak)) void adc_enable_dma(uint32_t adc)
{
    check_expected(adc);
}

__attribute__((weak)) void adc_disable_dma(uint32_t adc)
{
}

__attribute__((weak)) bool adc_get_flag(uint32_t adc, uint32_t flag)
{
    mock_type(bool);
}

__attribute__((weak)) void adc_clear_flag(uint32_t adc, uint32_t flag)
{
}

__attribute__((weak)) void adc_set_sample_time(uint32_t adc, uint8_t channel, uint8_t time)
{
}

__attribute__((weak)) void adc_set_sample_time_on_all_channels(uint32_t adc, uint8_t time)
{
}

__attribute__((weak)) void adc_disable_external_trigger_regular(uint32_t adc)
{
}

__attribute__((weak)) void adc_disable_external_trigger_injected(uint32_t adc)
{
}

__attribute__((weak)) void adc_start_conversion_direct(uint32_t adc)
{
}

__attribute__((weak)) void adc_set_dual_mode(uint32_t mode)
{
}

__attribute__((weak)) void adc_enable_temperature_sensor(void)
{
}

__attribute__((weak)) void adc_disable_temperature_sensor(void)
{
}

__attribute__((weak)) void adc_enable_external_trigger_regular(uint32_t adc, uint32_t trigger)
{
}

__attribute__((weak)) void adc_enable_external_trigger_injected(uint32_t adc, uint32_t trigger)
{
}

__attribute__((weak)) void adc_reset_calibration(uint32_t adc)
{
}

__attribute__((weak)) void adc_calibrate_async(uint32_t adc)
{
}

__attribute__((weak)) bool adc_is_calibrating(uint32_t adc)
{
    mock_type(bool);
}

__attribute__((weak)) void adc_calibrate(uint32_t adc)
{
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

