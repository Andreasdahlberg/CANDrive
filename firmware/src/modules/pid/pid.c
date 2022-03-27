/**
 * @file   pid.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  PID-controller module.
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

#include <assert.h>
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

static inline int32_t GetError(const struct pid_t *self_p, int32_t pv);
static inline int32_t GetIntegral(const struct pid_t *self_p, int32_t error);
static inline int32_t GetDerivative(const struct pid_t *self_p, int32_t input);
static inline int32_t LimitCV(const struct pid_t *self_p, int64_t cv);
static inline bool IsCVSaturated(const struct pid_t *self_p);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void PID_Init(struct pid_t *self_p)
{
    assert(self_p != NULL);

    *self_p = (__typeof__(*self_p)) {0};
}

int32_t PID_Update(struct pid_t *self_p, int32_t input)
{
    assert(self_p != NULL);

    const int32_t error = GetError(self_p, input);
    const int32_t integral = GetIntegral(self_p, error);
    const int32_t derivative = GetDerivative(self_p, input);

    const int32_t p = (error * self_p->parameters.kp);
    const int32_t i = (integral * self_p->parameters.ki);
    const int32_t d = (derivative * self_p->parameters.kd);

    const int64_t cv = (p + i - d + (self_p->parameters.scale / 2)) / self_p->parameters.scale;
    self_p->cv = LimitCV(self_p, cv);

    self_p->last_input = input;
    self_p->last_integral = integral;

    return self_p->cv;
}

void PID_SetSetpoint(struct pid_t *self_p, int32_t setpoint)
{
    assert(self_p != NULL);

    self_p->sp = setpoint;
}

int32_t PID_GetSetpoint(const struct pid_t *self_p)
{
    assert(self_p != NULL);

    return self_p->sp;
}

void PID_SetParameters(struct pid_t *self_p, const struct pid_parameters_t *parameters_p)
{
    assert(self_p != NULL);
    assert(parameters_p != NULL);
    assert(parameters_p->scale % 2 == 0);

    self_p->parameters = *parameters_p;
}

struct pid_parameters_t *PID_GetParameters(struct pid_t *self_p)
{
    assert(self_p != NULL);

    return &self_p->parameters;
}

int32_t PID_GetOutput(const struct pid_t *self_p)
{
    assert(self_p != NULL);

    return self_p->cv;
}

void PID_Reset(struct pid_t *self_p)
{
    assert(self_p != NULL);

    self_p->cv = 0;
    self_p->last_input = 0;
    self_p->last_integral = 0;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static inline int32_t GetError(const struct pid_t *self_p, int32_t pv)
{
    return self_p->sp - pv;
}

static inline int32_t GetIntegral(const struct pid_t *self_p, int32_t error)
{
    int32_t integral;

    if (!IsCVSaturated(self_p))
    {
        integral = self_p->last_integral + error;

        if (integral > self_p->parameters.imax)
        {
            integral = self_p->parameters.imax;
        }
        else if (integral < self_p->parameters.imin)
        {
            integral = self_p->parameters.imin;
        }
        else
        {
            /* No need to limit the integral if it's not saturated. */
        }
    }
    else
    {
        integral = self_p->last_integral;
    }

    return integral;
}

static inline int32_t GetDerivative(const struct pid_t *self_p, int32_t input)
{
    return input - self_p->last_input;
}

static inline int32_t LimitCV(const struct pid_t *self_p, int64_t cv)
{
    int32_t limited_cv;

    if (cv < self_p->parameters.cvmin)
    {
        limited_cv = self_p->parameters.cvmin;
    }
    else if (cv > self_p->parameters.cvmax)
    {
        limited_cv = self_p->parameters.cvmax;
    }
    else
    {
        limited_cv = (int32_t)cv;
    }

    return limited_cv;
}

static inline bool IsCVSaturated(const struct pid_t *self_p)
{
    return (self_p->cv <= self_p->parameters.cvmin) || (self_p->cv >= self_p->parameters.cvmax);
}
