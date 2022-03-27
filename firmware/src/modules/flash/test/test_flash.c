/**
 * @file   test_flash.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the flash helper module.
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
#include <string.h>
#include <libopencm3/stm32/flash.h>
#include "utility.h"
#include "logging.h"
#include "flash.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

/**
 * Since uint32_t is used for addresses in the target code, a virtual 32-bit
 * address is sent to the tested code and then translated to a 64-bit address
 * in the flash mock.
 */
#define FLASH_START_ADDRESS 0
#define NUMBER_OF_PAGES 4
#define PAGE_SIZE 1024

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct logging_logger_t *dummy_logger;
static uint8_t flash_data[NUMBER_OF_PAGES][PAGE_SIZE];

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//MOCKS
//////////////////////////////////////////////////////////////////////////

void flash_program_word(uint32_t address, uint32_t data)
{
    uint8_t *destination_p = ((uint8_t *)flash_data) + address;
    memcpy(destination_p, &data, sizeof(data));
}

void flash_erase_page(uint32_t page_address)
{
    uint32_t page_index = page_address / PAGE_SIZE;
    memset(flash_data[page_index], 0xFF, PAGE_SIZE);
}

void flash_clear_status_flags(void)
{
    function_called();
}

void flash_lock(void)
{
    function_called();
}

void flash_unlock(void)
{
    function_called();
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static int Setup(void **state)
{
    will_return_always(Logging_GetLogger, dummy_logger);
    expect_function_call(flash_clear_status_flags);
    Flash_Init();
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_Flash_Init(void **state)
{
    will_return_always(Logging_GetLogger, dummy_logger);
    expect_function_call(flash_clear_status_flags);
    Flash_Init();
}

static void test_Flash_Write_ZeroBytes(void **state)
{
    const uint32_t address = 0x00;
    const uint32_t data;

    expect_function_call(flash_unlock);
    expect_function_call(flash_lock);

    assert_true(Flash_Write(address, &data, 0));
}

static void test_Flash_Write_CompleteWords(void **state)
{
    const uint32_t address = 0x00;
    const uint32_t data[2] = {0xAABBCCDD, 0xFFEEDDCC};

    expect_function_call(flash_unlock);
    will_return_count(flash_get_status_flags, FLASH_SR_EOP, ElementsIn(data));
    expect_function_calls(flash_clear_status_flags, ElementsIn(data));
    expect_function_call(flash_lock);

    assert_true(Flash_Write(address, &data, sizeof(data)));
    assert_memory_equal(flash_data, data, sizeof(data));
}

static void test_Flash_Write_Remainder(void **state)
{
    const uint32_t address = 0x00;
    const uint8_t data[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    const uint32_t number_of_words = sizeof(data) / sizeof(uint32_t);

    expect_function_call(flash_unlock);
    will_return_count(flash_get_status_flags, FLASH_SR_EOP, number_of_words + 1);
    expect_function_calls(flash_clear_status_flags, number_of_words + 1);
    expect_function_call(flash_lock);

    assert_true(Flash_Write(address, &data, sizeof(data)));
    assert_memory_equal(flash_data, data, sizeof(data));
}

static void test_Flash_Write_Failed(void **state)
{
    const uint32_t address = 0x00;
    const uint32_t data[1] = {0xAABBCCDD};

    expect_function_call(flash_unlock);
    will_return_always(flash_get_status_flags, FLASH_SR_PGERR);
    expect_function_call(flash_clear_status_flags);
    expect_function_call(flash_lock);

    assert_false(Flash_Write(address, &data, sizeof(data)));
}

static void test_Flash_ErasePage(void **state)
{
    will_return_always(flash_get_status_flags, FLASH_SR_EOP);

    flash_data[0][0] = 0xAA;
    flash_data[1][0] = 0xBB;
    expect_function_call(flash_unlock);
    expect_function_call(flash_clear_status_flags);
    expect_function_call(flash_lock);
    assert_true(Flash_ErasePage(0x000));
    assert_int_equal(flash_data[0][0], 0xFF);
    assert_int_equal(flash_data[1][0], 0xBB);

    flash_data[0][0] = 0xAA;
    expect_function_call(flash_unlock);
    expect_function_call(flash_clear_status_flags);
    expect_function_call(flash_lock);
    assert_true(Flash_ErasePage(0x400));
    assert_int_equal(flash_data[0][0], 0xAA);
    assert_int_equal(flash_data[1][0], 0xFF);
}

static void test_Flash_ErasePage_Failed(void **state)
{
    will_return_always(flash_get_status_flags, FLASH_SR_PGERR);

    expect_function_call(flash_unlock);
    expect_function_call(flash_clear_status_flags);
    expect_function_call(flash_lock);
    assert_false(Flash_ErasePage(0x000));
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_flash[] =
    {
        cmocka_unit_test(test_Flash_Init),
        cmocka_unit_test_setup(test_Flash_Write_ZeroBytes, Setup),
        cmocka_unit_test_setup(test_Flash_Write_CompleteWords, Setup),
        cmocka_unit_test_setup(test_Flash_Write_Remainder, Setup),
        cmocka_unit_test_setup(test_Flash_Write_Failed, Setup),
        cmocka_unit_test_setup(test_Flash_ErasePage, Setup),
        cmocka_unit_test_setup(test_Flash_ErasePage_Failed, Setup),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_flash, NULL, NULL);
}
