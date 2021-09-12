/**
 * @file   mock_motor.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for motor.
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
#include "motor.h"

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

__attribute__((weak)) void Motor_Init(struct motor_t *self_p, const char *name, const struct motor_config_t *config_p)
{
    assert_non_null(self_p);
    check_expected_ptr(config_p);
}

__attribute__((weak)) void Motor_Update(struct motor_t *self_p)
{
    assert_non_null(self_p);
    function_called();
}

__attribute__((weak)) int16_t Motor_GetRPM(const struct motor_t *self_p)
{
    assert_non_null(self_p);
    return mock_type(int16_t);
}

__attribute__((weak)) int16_t Motor_GetCurrent(const struct motor_t *self_p)
{
    assert_non_null(self_p);
    return mock_type(int16_t);
}

__attribute__((weak)) void Motor_SetSpeed(struct motor_t *self_p, int16_t speed)
{
    assert_non_null(self_p);
    check_expected(speed);
}

__attribute__((weak)) void Motor_Run(struct motor_t *self_p)
{
    assert_non_null(self_p);
    function_called();
}

__attribute__((weak)) void Motor_Coast(struct motor_t *self_p)
{
    assert_non_null(self_p);
    function_called();
}

__attribute__((weak)) void Motor_Brake(struct motor_t *self_p)
{
    assert_non_null(self_p);
    function_called();
}

__attribute__((weak)) enum motor_status_t Motor_GetStatus(const struct motor_t *self_p)
{
    assert_non_null(self_p);
    return mock_type(enum motor_status_t);
}

__attribute__((weak)) enum motor_direction_t Motor_GetDirection(const struct motor_t *self_p)
{
    assert_non_null(self_p);
    return mock_type(enum motor_direction_t);
}

__attribute__((weak)) uint32_t Motor_GetPosition(const struct motor_t *self_p)
{
    assert_non_null(self_p);
    return mock_type(uint32_t);
}

__attribute__((weak)) const char *Motor_DirectionToString(const struct motor_t *self_p, enum motor_direction_t direction)
{
    assert_non_null(self_p);
    return mock_type(char *);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
