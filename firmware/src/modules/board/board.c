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

#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/scb.h>
#include <assert.h>
#include "memory_map.h"
#include "utility.h"
#include "board.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define NUMBER_OF_MOTORS 1
#define GPIO_STATUS_LED_CLOCK RCC_GPIOA
#define GPIO_STATUS_LED_PORT GPIOA
#define GPIO_STATUS_LED GPIO5
#define GPIO_EMERGENCY_CLOCK RCC_GPIOC
#define GPIO_EMERGENCY_PORT GPIOC
#define GPIO_EMERGENCY GPIO13

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct module_t
{
    const struct motor_config_t motor_configs[NUMBER_OF_MOTORS];
    uint32_t reset_flags;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct module_t module =
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
    },
    0
};

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static void StoreResetFlags(void);
static void ClearResetFlags(void);
static void SetupClock(void);
static void SetupGPIO(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void Board_Init(void)
{
    StoreResetFlags();
    ClearResetFlags();
    SetupClock();
    SetupGPIO();
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

size_t Board_GetMaxNumberOfMotors(void)
{
    return ElementsIn(module.motor_configs);
}

void Board_ToggleStatusLED(void)
{
    gpio_toggle(GPIO_STATUS_LED_PORT, GPIO_STATUS_LED);
}

uint32_t Board_GetResetFlags(void)
{
    return module.reset_flags;
}

void Board_Reset(void)
{
    scb_reset_system();
}

bool Board_GetEmergencyPinState(void)
{
    return !(bool)gpio_get(GPIO_EMERGENCY_PORT, GPIO_EMERGENCY);
}

uintptr_t Board_GetNVSAddress(void)
{
    const uintptr_t *nvs = &__nvsrom_start__;
    return (uintptr_t)nvs;
}

uintptr_t Board_GetApplicationAddress(void)
{
    const uintptr_t *app = &__approm_start__;
    return (uintptr_t)app;
}


uint32_t Board_GetNumberOfPagesInNVS(void)
{
    const uint32_t sector_size = 1024;
    const uintptr_t *nvs = &__nvsrom_size__;

    assert((uintptr_t)nvs % sector_size == 0);
    return (uintptr_t)nvs / sector_size;
}

uint32_t Board_GetMaxCurrent(void)
{
    return 5000;
}

uintptr_t Board_GetBackupMemoryAddress(void)
{
    return (BACKUP_REGS_BASE + 0x04);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static void StoreResetFlags(void)
{
    module.reset_flags = RCC_CSR & (uint32_t)RCC_CSR_RESET_FLAGS;
}

static void ClearResetFlags(void)
{
    RCC_CSR |= RCC_CSR_RMVF;
}

static void SetupClock(void)
{
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
}

static void SetupGPIO(void)
{
    rcc_periph_clock_enable(GPIO_STATUS_LED_CLOCK);
    gpio_set(GPIO_STATUS_LED_PORT, GPIO_STATUS_LED);
    gpio_set_mode(GPIO_STATUS_LED_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO_STATUS_LED);

    rcc_periph_clock_enable(GPIO_EMERGENCY_CLOCK);
    gpio_set(GPIO_EMERGENCY_PORT, GPIO_EMERGENCY);
    gpio_set_mode(GPIO_EMERGENCY_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO_EMERGENCY);
}
