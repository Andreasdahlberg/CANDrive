/**
 * @file   board.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Board support module.
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

#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/gpio.h>
#include <assert.h>
#include "utility.h"
#include "board.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define NUMBER_OF_MOTORS 1

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct module_t
{
    struct motor_config_t motor_configs[NUMBER_OF_MOTORS];
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static const struct module_t module =
{
    .motor_configs = {
        {
            .pwm = {
                .timer_peripheral = TIM3,
                .remap = AFIO_MAPR_TIM3_REMAP_FULL_REMAP,
                .gpio_port = GPIOC,
                .gpio = GPIO8,
                .oc_id = TIM_OC3,
                .peripheral_clocks = {RCC_GPIOC, RCC_TIM3, RCC_AFIO}
            },
            .driver = {
                .port = GPIOC,
                .sel = GPIO0,
                .cs = GPIO1,
                .ina = GPIO2,
                .inb = GPIO3,
                .gpio_clock = RCC_GPIOC
            },
            .encoder = {
                .port = GPIOB,
                .a = GPIO6,
                .b = GPIO7,
                .gpio_clock = RCC_GPIOB,
                .timer = TIM4,
                .timer_clock = RCC_TIM4,
                .timer_rst = RST_TIM4
            },
            .adc = {
                .channel = 11
            }
        },
    }
};

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static void SetupClock(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void Board_Init(void)
{
    SetupClock();
}

uint32_t Board_GetHardwareRevision(void)
{
    return 0;
}

uint32_t Board_GetSoftwareRevision(void)
{
    return 0;
}

struct board_id_t Board_GetId(void)
{
    const uint32_t uid_base_address = 0x1FFFF7E8;

    struct board_id_t id;
    id.offset_0 = MMIO32(uid_base_address);
    id.offset_4 = MMIO32(uid_base_address + 4);
    id.offset_8 = MMIO32(uid_base_address + 8);

    return id;
}

const struct motor_config_t *Board_GetMotorConfig(size_t index)
{
    assert(index < ElementsIn(module.motor_configs));

    return &module.motor_configs[index];
}

size_t Board_GetNumberOfMotors(void)
{
    return ElementsIn(module.motor_configs);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static void SetupClock(void)
{
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
}
