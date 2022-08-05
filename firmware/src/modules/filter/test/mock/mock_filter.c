/**
 * @file   mock_filter.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for filter.
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
#include <stdbool.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "filter.h"

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

__attribute__((weak)) void Filter_Init(struct filter_t *self_p, uint32_t initial_value, uint32_t alpha)
{
    assert_non_null(self_p);
    assert_int_not_equal(alpha, 0);
}

__attribute__((weak)) void Filter_Process(struct filter_t *self_p, uint32_t sample)
{
    assert_non_null(self_p);
}

__attribute__((weak)) uint32_t Filter_Output(const struct filter_t *self_p)
{
    assert_non_null(self_p);
    return mock_type(uint32_t);
}

__attribute__((weak)) bool Filter_IsInitialized(const struct filter_t *self_p)
{
    assert_non_null(self_p);
    return mock_type(bool);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

