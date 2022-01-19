/**
 * @file   main.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  CANDrive bootloader.
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

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/scb.h>
#include "memory_map.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION __FILE__ ":" S2(__LINE__)
#define LOG_MESSAGE(text) USARTSend("[0] INFO:Boot " LOCATION " " text "\r\n")

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static inline void ClockSetup(void);
static inline void GPIOSetup(void);
static inline void USARTSetup(uint32_t baud_rate);
static void USARTSend(const char *text_p);
static inline void StartApplication(void) __attribute__((noreturn));
static inline void PrepareForApplication(void);
static void JumpToApplication(__attribute__((unused)) uint32_t pc, __attribute__((unused)) uint32_t sp) __attribute__((naked, noreturn));

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(void)
{
    ClockSetup();
    GPIOSetup();
    USARTSetup(BAUD_RATE);

    LOG_MESSAGE("boot_info: {sw: "GIT_DESC"}");

    StartApplication();
    while (1)
    {
        /* Should never reach this. */
    }
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static inline void ClockSetup(void)
{
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

    /* Enable clocks for GPIO port A (for GPIO_USART2_TX) and USART2. */
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_AFIO);
    rcc_periph_clock_enable(RCC_USART2);
}

static inline void GPIOSetup(void)
{
    /* Setup GPIO pin GPIO_USART2_RE_TX on GPIO port B for transmit. */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);
}

static inline void USARTSetup(uint32_t baud_rate)
{
    usart_set_baudrate(USART2, baud_rate);
    usart_set_databits(USART2, 8);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
    usart_set_mode(USART2, USART_MODE_TX_RX);
    usart_enable(USART2);
}

static void USARTSend(const char *text_p)
{
    const size_t length = strlen(text_p);
    for (size_t i = 0; i < length; ++i)
    {
        usart_send_blocking(USART2, text_p[i]);
    }
}

static inline void StartApplication(void)
{
    const uint32_t *app_code = &__approm_start__;
    const uint32_t app_sp = *app_code;
    const uint32_t app_start = *(app_code + 1);

    LOG_MESSAGE("Start application");
    PrepareForApplication();
    JumpToApplication(app_start, app_sp);
}

static inline void PrepareForApplication(void)
{
    usart_disable(USART2);
    rcc_periph_clock_disable(RCC_GPIOA);
    rcc_periph_clock_disable(RCC_AFIO);
    rcc_periph_clock_disable(RCC_USART2);

    /* Switch to application vector table. */
    const uint32_t *app_code = &__approm_start__;
    const uint32_t app_vector_table = (uint32_t)app_code;
    SCB_VTOR = app_vector_table;
}

static void JumpToApplication(__attribute__((unused)) uint32_t pc, __attribute__((unused)) uint32_t sp)
{
    __asm("MSR MSP,r1");
    __asm("BX r0");
}
