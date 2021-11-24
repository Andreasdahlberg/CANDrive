/**
 * @file   test_nvs.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the NVS module.
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
#include <string.h>
#include <stdbool.h>
#include <libopencm3/stm32/flash.h>
#include "nvs.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

/**
 * Since uint32_t is used for addresses in the target code, a virtual 32-bit
 * address is sent to the tested code and then translated to a 64-bit address
 * in the flash mock.
 */
#define FLASH_START_ADDRESS 0
#define NUMBER_OF_PAGES 2
#define PAGE_SIZE 1024

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct logging_logger_t *dummy_logger;
static bool create_corrupt_crc = false;
static uint32_t flash_data[NUMBER_OF_PAGES][PAGE_SIZE];

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static int Setup(void **state)
{
    create_corrupt_crc = false;

    flash_erase_all_pages();
    will_return_maybe(Logging_GetLogger, dummy_logger);
    will_return_maybe(flash_get_status_flags, FLASH_SR_EOP);

    NVS_Init(FLASH_START_ADDRESS, NUMBER_OF_PAGES);

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//MOCKS
//////////////////////////////////////////////////////////////////////////

void *__real_memcpy (void *destination_p, const void *source_p, size_t length);

void flash_program_word(uint32_t address, uint32_t data)
{
    uint8_t *destination_p = ((uint8_t *)flash_data) + address;
    __real_memcpy(destination_p, &data, sizeof(data));
}

void flash_program_half_word(uint32_t address, uint16_t data)
{
    uint8_t *destination_p = ((uint8_t *)flash_data) + address;
    __real_memcpy(destination_p, &data, sizeof(data));
}

void *__wrap_memcpy (void *destination_p, const void *source_p, size_t length)
{
    uint8_t *real_source_p = ((uint8_t *)flash_data) + ((uintptr_t )source_p);
    return __real_memcpy(destination_p, real_source_p, length);
}

void flash_erase_page(uint32_t page_address)
{
    uint32_t page_index = page_address / PAGE_SIZE;
    memset(flash_data[page_index], 0xFF, PAGE_SIZE);
}

void flash_erase_all_pages(void)
{
    memset(flash_data, 0xFF, sizeof(flash_data));
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_NVS_Init_Invalid(void **state)
{
    expect_assert_failure(NVS_Init(FLASH_START_ADDRESS, 0));
    expect_assert_failure(NVS_Init(FLASH_START_ADDRESS, 1));
}

static void test_NVS_StoreAndRetrieve(void **state)
{
    will_return_maybe(flash_get_status_flags, FLASH_SR_EOP);

    assert_true(NVS_Store("Foo", 10));
    assert_true(NVS_Store("Bar", 20));
    assert_true(NVS_Store("Foo", 30));

    uint32_t value;
    assert_true(NVS_Retrieve("Foo", &value));
    assert_int_equal(value, 30);
    assert_true(NVS_Retrieve("Bar", &value));
    assert_int_equal(value, 20);
    assert_false(NVS_Retrieve("Foobar", &value));
}

static void test_NVS_Store_Failed(void **state)
{
    will_return_maybe(flash_get_status_flags, FLASH_SR_PGERR);

    assert_false(NVS_Store("Foo", 10));
}

static void test_NVS_Remove(void **state)
{
    will_return_maybe(flash_get_status_flags, FLASH_SR_EOP);

    /* Try to remove non existing value. */
    assert_false(NVS_Remove("Foo"));

    /* Removing existing value. */
    uint32_t value;
    assert_true(NVS_Store("Bar", 10));
    assert_true(NVS_Remove("Bar"));
    assert_false(NVS_Retrieve("Bar", &value));

    /* Try to remove already removed value. */
    assert_false(NVS_Remove("Bar"));

    /* Remove value with several old values. */
    assert_true(NVS_Store("Foo", 10));
    assert_true(NVS_Store("Foo", 20));
    assert_true(NVS_Store("Foo", 30));
    assert_true(NVS_Remove("Foo"));
    assert_false(NVS_Retrieve("Foo", &value));
}

static void test_NVS_Remove_FailedFlashWrite(void **state)
{
    will_return_count(flash_get_status_flags, FLASH_SR_EOP, 8);
    assert_true(NVS_Store("Foo", 10));

    will_return_maybe(flash_get_status_flags, FLASH_SR_PGERR);
    assert_false(NVS_Remove("Foo"));
}

static void test_NVS_RetrieveExistingValues(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    will_return_maybe(flash_get_status_flags, FLASH_SR_EOP);

    assert_true(NVS_Store("Foo", 50));
    assert_true(NVS_Store("Bar", 60));
    assert_true(NVS_Store("Bar", 70));

    NVS_Init(FLASH_START_ADDRESS, NUMBER_OF_PAGES);

    uint32_t value;
    assert_true(NVS_Retrieve("Foo", &value));
    assert_int_equal(value, 50);
    assert_true(NVS_Retrieve("Bar", &value));
    assert_int_equal(value, 70);
    assert_false(NVS_Retrieve("Foobar", &value));
}

static void test_NVS_PageFull(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    will_return_maybe(flash_get_status_flags, FLASH_SR_EOP);

    /* Fill up the first page */
    for (size_t i = 0; i < 16; ++i)
    {
        assert_true(NVS_Store("A", i));
        assert_true(NVS_Store("B", i));
        assert_true(NVS_Store("C", i));
        assert_true(NVS_Store("D", i));
    }

    /* Verify that the correct values was moved to the new page. */
    uint32_t value;
    assert_true(NVS_Retrieve("A", &value));
    assert_int_equal(value, 15);
    assert_true(NVS_Retrieve("B", &value));
    assert_int_equal(value, 15);
    assert_true(NVS_Retrieve("C", &value));
    assert_int_equal(value, 15);
    assert_true(NVS_Retrieve("D", &value));
    assert_int_equal(value, 15);

    /* Verify that a new value is stored correctly on the new page. */
    assert_true(NVS_Store("D", 16));
    assert_true(NVS_Retrieve("D", &value));
    assert_int_equal(value, 16);
    assert_true(NVS_Retrieve("A", &value));
    assert_int_equal(value, 15);

    /* Verify that the correct page is used after init. */
    NVS_Init(FLASH_START_ADDRESS, NUMBER_OF_PAGES);
    assert_true(NVS_Retrieve("B", &value));
    assert_int_equal(value, 15);
    assert_true(NVS_Retrieve("D", &value));
    assert_int_equal(value, 16);
}

static void test_NVS_PageWrapAround(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    will_return_maybe(flash_get_status_flags, FLASH_SR_EOP);

    /* Fill up the first page. */
    for (size_t i = 0; i < 16; ++i)
    {
        assert_true(NVS_Store("A", i));
        assert_true(NVS_Store("B", i));
        assert_true(NVS_Store("C", i));
        assert_true(NVS_Store("D", i));
    }

    /* Fill up the second page. */
    for (size_t i = 16; i < 32; ++i)
    {
        assert_true(NVS_Store("A", i));
        assert_true(NVS_Store("B", i));
        assert_true(NVS_Store("C", i));
        assert_true(NVS_Store("D", i));
    }

    /* Verify that the correct values was moved to the new page. */
    uint32_t value;
    assert_true(NVS_Retrieve("A", &value));
    assert_int_equal(value, 31);
    assert_true(NVS_Retrieve("B", &value));
    assert_int_equal(value, 31);
    assert_true(NVS_Retrieve("C", &value));
    assert_int_equal(value, 31);
    assert_true(NVS_Retrieve("D", &value));
    assert_int_equal(value, 31);

    /*Verify that a new value is stored correctly on the new page. */
    assert_true(NVS_Store("C", 32));
    assert_true(NVS_Retrieve("C", &value));
    assert_int_equal(value, 32);
    assert_true(NVS_Retrieve("B", &value));
    assert_int_equal(value, 31);

    /* Verify that the correct page is used after init. */
    NVS_Init(FLASH_START_ADDRESS, NUMBER_OF_PAGES);
    assert_true(NVS_Retrieve("C", &value));
    assert_int_equal(value, 32);
    assert_true(NVS_Retrieve("A", &value));
    assert_int_equal(value, 31);
}

static void test_NVS_Clear(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    will_return_maybe(flash_get_status_flags, FLASH_SR_EOP);

    assert_true(NVS_Store("Foo", 10));
    assert_true(NVS_Store("Bar", 20));
    assert_true(NVS_Clear());

    uint32_t value;
    assert_false(NVS_Retrieve("Foo", &value));
    assert_false(NVS_Retrieve("Bar", &value));

    assert_true(NVS_Store("Foo", 30));
    assert_true(NVS_Retrieve("Foo", &value));
    assert_int_equal(value, 30);
}

static void test_NVS_ClearFailed(void **state)
{
    will_return_maybe(Logging_GetLogger, dummy_logger);
    will_return_maybe(flash_get_status_flags, FLASH_SR_PGERR);

    assert_false(NVS_Clear());
}

/////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_nvs[] =
    {
        cmocka_unit_test(test_NVS_Init_Invalid),
        cmocka_unit_test_setup(test_NVS_StoreAndRetrieve, Setup),
        cmocka_unit_test_setup(test_NVS_Store_Failed, Setup),
        cmocka_unit_test_setup(test_NVS_Remove, Setup),
        cmocka_unit_test_setup(test_NVS_Remove_FailedFlashWrite, Setup),
        cmocka_unit_test_setup(test_NVS_RetrieveExistingValues, Setup),
        cmocka_unit_test_setup(test_NVS_PageFull, Setup),
        cmocka_unit_test_setup(test_NVS_PageWrapAround, Setup),
        cmocka_unit_test_setup(test_NVS_Clear, Setup),
        cmocka_unit_test_setup(test_NVS_ClearFailed, Setup),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_nvs, NULL, NULL);
}

uint32_t CRC_Calculate(const void *data_p, size_t length)
{
    uint32_t crc = 0xAAAAAAAA;

    for(size_t i = 0; i < length; ++i)
    {
        crc += ((uint8_t *)data_p)[i];
    }

    if (create_corrupt_crc)
    {
        crc += 1;
    }

    return crc;
}
