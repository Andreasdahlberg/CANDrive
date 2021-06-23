
/**
 * @file   main.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Main application
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
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <stdio.h>
#include <string.h>
#include "serial.h"
#include "logging.h"
#include "can_interface.h"

static void clock_setup(void)
{
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

    /* Enable GPIOA clock (for LED GPIOs). */
    rcc_periph_clock_enable(RCC_GPIOC);
}

static void gpio_setup(void)
{
    gpio_set(GPIOA, GPIO5);

    /* Setup GPIO6 and 7 (in GPIO port A) for LED use. */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, GPIO5);
}

static inline void nop(void)
{
    __asm__("nop");
}

static uint32_t GetTimestamp(void)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(void)
{
    clock_setup();
    gpio_setup();
    Serial_Init(BAUD_RATE);
    Logging_Init(GetTimestamp);
    CANInterface_Init();

    uint32_t a = 0;
    while (1)
    {
        gpio_toggle(GPIOA, GPIO5);
        printf("Hello %lu!\r\n", a);
        ++a;

        for (size_t i = 0; i < 800000; ++i)    /* Wait a bit. */
        {
            nop();
        }
    }

    return 0;
}
