/**
 * @file   mock_gpio.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for gpio.
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
#include <libopencm3/stm32/gpio.h>

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

__attribute__((weak)) void gpio_set_mode(uint32_t gpioport, uint8_t mode, uint8_t cnf, uint16_t gpios)
{
}

__attribute__((weak)) void gpio_clear(uint32_t gpioport, uint16_t gpios)
{

}

__attribute__((weak)) void gpio_set(uint32_t gpio_port, uint16_t gpios)
{

}

__attribute__((weak)) void gpio_toggle(uint32_t gpio_port, uint16_t gpios)
{
    function_called();
}

__attribute__((weak)) uint16_t gpio_get(uint32_t gpio_port, uint16_t gpios)
{
    return mock_type(uint16_t);
}

__attribute__((weak)) void gpio_set_eventout(uint8_t evoutport, uint8_t evoutpin)
{
}

__attribute__((weak)) void gpio_primary_remap(uint32_t swjenable, uint32_t maps)
{
}

__attribute__((weak)) void gpio_secondary_remap(uint32_t maps)
{
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
