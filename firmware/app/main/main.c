
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
#include "adc.h"
#include "systime.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define MAIN_LOGGER_NAME "Main"
#ifndef MAIN_LOGGER_DEBUG_LEVEL
#define MAIN_LOGGER_DEBUG_LEVEL LOGGING_DEBUG
#endif

#define GPIO_LED_PORT GPIOA
#define GPIO_LED GPIO5

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static void SetupClock(void);
static void SetupGPIO(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(void)
{
    SetupClock();
    SetupGPIO();

    SysTime_Init();
    Serial_Init(BAUD_RATE);
    Logging_Init(SysTime_GetSystemTime);
    CANInterface_Init();
    ADC_Init();

    logging_logger_t *logger_p = Logging_GetLogger(MAIN_LOGGER_NAME);
    Logging_SetLevel(logger_p, MAIN_LOGGER_DEBUG_LEVEL);
    Logging_Info(logger_p, "Application ready");

    uint32_t led_time = SysTime_GetSystemTime();
    while (1)
    {
        if (SysTime_GetDifference(led_time) >= 1000)
        {
            gpio_toggle(GPIO_LED_PORT, GPIO_LED);
            led_time = SysTime_GetSystemTime();
        }
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static void SetupClock(void)
{
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

    /* Enable GPIO clock, for LED GPIO. */
    rcc_periph_clock_enable(RCC_GPIOA);
}

static void SetupGPIO(void)
{
    gpio_set(GPIO_LED_PORT, GPIO_LED);
    gpio_set_mode(GPIO_LED_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO5);
}
