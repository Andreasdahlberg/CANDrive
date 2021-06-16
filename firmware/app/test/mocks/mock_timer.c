/**
 * @file   mock_timer.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for timer.
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
#include <libopencm3/stm32/timer.h>

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

__attribute__((weak)) void timer_enable_irq(uint32_t timer_peripheral, uint32_t irq)
{
}

__attribute__((weak)) void timer_disable_irq(uint32_t timer_peripheral, uint32_t irq)
{
}

__attribute__((weak)) bool timer_interrupt_source(uint32_t timer_peripheral, uint32_t flag)
{
    mock_type(bool);
}

__attribute__((weak)) bool timer_get_flag(uint32_t timer_peripheral, uint32_t flag)
{
    mock_type(bool);
}

__attribute__((weak)) void timer_clear_flag(uint32_t timer_peripheral, uint32_t flag)
{
}

__attribute__((weak)) void timer_set_mode(uint32_t timer_peripheral, uint32_t clock_div, uint32_t alignment, uint32_t direction)
{
}

__attribute__((weak)) void timer_set_clock_division(uint32_t timer_peripheral, uint32_t clock_div)
{
}

__attribute__((weak)) void timer_enable_preload(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_disable_preload(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_set_alignment(uint32_t timer_peripheral, uint32_t alignment)
{
}

__attribute__((weak)) void timer_direction_up(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_direction_down(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_one_shot_mode(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_continuous_mode(uint32_t timer_peripheral)
{
    check_expected(timer_peripheral);
}

__attribute__((weak)) void timer_update_on_any(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_update_on_overflow(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_enable_update_event(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_disable_update_event(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_enable_counter(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_disable_counter(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_set_output_idle_state(uint32_t timer_peripheral, uint32_t outputs)
{
}

__attribute__((weak)) void timer_reset_output_idle_state(uint32_t timer_peripheral, uint32_t outputs)
{
}

__attribute__((weak)) void timer_set_ti1_ch123_xor(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_set_ti1_ch1(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_set_master_mode(uint32_t timer_peripheral, uint32_t mode)
{
}

__attribute__((weak)) void timer_set_dma_on_compare_event(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_set_dma_on_update_event(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_enable_compare_control_update_on_trigger(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_disable_compare_control_update_on_trigger(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_enable_preload_complementry_enable_bits(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_disable_preload_complementry_enable_bits(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_set_prescaler(uint32_t timer_peripheral, uint32_t value)
{
}

__attribute__((weak)) void timer_set_repetition_counter(uint32_t timer_peripheral, uint32_t value)
{
}

__attribute__((weak)) void timer_set_period(uint32_t timer_peripheral, uint32_t period)
{
    check_expected(timer_peripheral);
    check_expected(period);
}

__attribute__((weak)) void timer_enable_oc_clear(uint32_t timer_peripheral, enum tim_oc_id oc_id)
{
}

__attribute__((weak)) void timer_disable_oc_clear(uint32_t timer_peripheral, enum tim_oc_id oc_id)
{
}

__attribute__((weak)) void timer_set_oc_fast_mode(uint32_t timer_peripheral, enum tim_oc_id oc_id)
{
}

__attribute__((weak)) void timer_set_oc_slow_mode(uint32_t timer_peripheral, enum tim_oc_id oc_id)
{
}

__attribute__((weak)) void timer_set_oc_mode(uint32_t timer_peripheral, enum tim_oc_id oc_id, enum tim_oc_mode oc_mode)
{
    check_expected(timer_peripheral);
    check_expected(oc_mode);
}

__attribute__((weak)) void timer_enable_oc_preload(uint32_t timer_peripheral, enum tim_oc_id oc_id)
{
}

__attribute__((weak)) void timer_disable_oc_preload(uint32_t timer_peripheral, enum tim_oc_id oc_id)
{
}

__attribute__((weak)) void timer_set_oc_polarity_high(uint32_t timer_peripheral, enum tim_oc_id oc_id)
{
}

__attribute__((weak)) void timer_set_oc_polarity_low(uint32_t timer_peripheral, enum tim_oc_id oc_id)
{
}

__attribute__((weak)) void timer_enable_oc_output(uint32_t timer_peripheral, enum tim_oc_id oc_id)
{
}

__attribute__((weak)) void timer_disable_oc_output(uint32_t timer_peripheral, enum tim_oc_id oc_id)
{
}

__attribute__((weak)) void timer_set_oc_idle_state_set(uint32_t timer_peripheral, enum tim_oc_id oc_id)
{
}

__attribute__((weak)) void timer_set_oc_idle_state_unset(uint32_t timer_peripheral, enum tim_oc_id oc_id)
{
}

__attribute__((weak)) void timer_set_oc_value(uint32_t timer_peripheral, enum tim_oc_id oc_id, uint32_t value)
{
}

__attribute__((weak)) void timer_enable_break_main_output(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_disable_break_main_output(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_enable_break_automatic_output(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_disable_break_automatic_output(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_set_break_polarity_high(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_set_break_polarity_low(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_enable_break(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_disable_break(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_set_enabled_off_state_in_run_mode(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_set_disabled_off_state_in_run_mode(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_set_enabled_off_state_in_idle_mode(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_set_disabled_off_state_in_idle_mode(uint32_t timer_peripheral)
{
}

__attribute__((weak)) void timer_set_break_lock(uint32_t timer_peripheral, uint32_t lock)
{
}

__attribute__((weak)) void timer_set_deadtime(uint32_t timer_peripheral, uint32_t deadtime)
{
}

__attribute__((weak)) void timer_generate_event(uint32_t timer_peripheral, uint32_t event)
{
}

__attribute__((weak)) uint32_t timer_get_counter(uint32_t timer_peripheral)
{
    mock_type(uint32_t);
}

__attribute__((weak)) void timer_set_counter(uint32_t timer_peripheral, uint32_t count)
{
}

__attribute__((weak)) void timer_ic_set_filter(uint32_t timer, enum tim_ic_id ic, enum tim_ic_filter flt)
{
}

__attribute__((weak)) void timer_ic_set_prescaler(uint32_t timer, enum tim_ic_id ic, enum tim_ic_psc psc)
{
}

__attribute__((weak)) void timer_ic_set_input(uint32_t timer, enum tim_ic_id ic, enum tim_ic_input in)
{
}

__attribute__((weak)) void timer_ic_enable(uint32_t timer, enum tim_ic_id ic)
{
}

__attribute__((weak)) void timer_ic_disable(uint32_t timer, enum tim_ic_id ic)
{
}

__attribute__((weak)) void timer_slave_set_filter(uint32_t timer, enum tim_ic_filter flt)
{
}

__attribute__((weak)) void timer_slave_set_prescaler(uint32_t timer, enum tim_ic_psc psc)
{
}

__attribute__((weak)) void timer_slave_set_polarity(uint32_t timer, enum tim_et_pol pol)
{
}

__attribute__((weak)) void timer_slave_set_mode(uint32_t timer, uint8_t mode)
{
}

__attribute__((weak)) void timer_slave_set_trigger(uint32_t timer, uint8_t trigger)
{
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
