/**
 * @file   test_image.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the Image module.
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
#include "image.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct dummy_image_t
{
    struct image_header_t header;
    uint8_t data[128];
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct dummy_image_t image;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static int Setup(void **state)
{
    image = (__typeof__(image)) {0};

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

void test_Image_GetHeader_InvalidHeader(void **state)
{
    assert_null(Image_GetHeader((uintptr_t *)&image));
}

void test_Image_GetHeader(void **state)
{
    image.header.header_magic = IMAGE_HEADER_MAGIC;
    assert_ptr_equal(Image_GetHeader((uintptr_t *)&image), &image.header);
}

void test_Image_IsValid_Invalid(void **state)
{
    assert_false(Image_IsValid((uintptr_t *)&image));

    image.header.header_magic = IMAGE_HEADER_MAGIC;
    assert_false(Image_IsValid((uintptr_t *)&image));
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_image[] =
    {
        cmocka_unit_test_setup(test_Image_GetHeader_InvalidHeader, Setup),
        cmocka_unit_test_setup(test_Image_GetHeader, Setup),
        cmocka_unit_test_setup(test_Image_IsValid_Invalid, Setup)
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_image, NULL, NULL);
}
