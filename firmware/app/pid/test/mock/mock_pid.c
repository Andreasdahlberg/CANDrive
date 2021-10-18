/**
 * @file   mock_pid.c
 * @Author Andreas Dahlberg
 * @brief  Mock functions for pid.
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
#include "pid.h"

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

__attribute__((weak)) void PID_Init(struct pid_t *self_p)
{
    assert_non_null(self_p);
}

__attribute__((weak)) int32_t PID_Update(struct pid_t *self_p, int32_t input)
{
    assert_non_null(self_p);
    return mock_type(int32_t);
}

__attribute__((weak)) void PID_SetSetpoint(struct pid_t *self_p, int32_t setpoint)
{
    assert_non_null(self_p);
    check_expected(setpoint);
}

__attribute__((weak)) int32_t PID_GetSetpoint(const struct pid_t *self_p)
{
    assert_non_null(self_p);
    return mock_type(int32_t);
}

__attribute__((weak)) void PID_SetParameters(struct pid_t *self_p, const struct pid_parameters_t *parameters_p)
{
    assert_non_null(self_p);
}

__attribute__((weak)) struct pid_parameters_t *PID_GetParameters(struct pid_t *self_p)
{
    assert_non_null(self_p);
    return mock_type(struct pid_parameters_t *);
}

__attribute__((weak)) int32_t PID_GetOutput(const struct pid_t *self_p)
{
    assert_non_null(self_p);
    return mock_type(int32_t);
}

__attribute__((weak)) void PID_Reset(struct pid_t *self_p)
{
    assert_non_null(self_p);
    function_called();
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

