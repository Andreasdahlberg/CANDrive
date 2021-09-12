/**
 * @file   motor_controller.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Motor manager module.
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
#include <stdio.h>
#include "utility.h"
#include "logging.h"
#include "board.h"
#include "systime.h"
#include "pid.h"
#include "motor_controller.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define MOTOR_CONTROLLER_LOGGER_NAME "MoCon"
#ifndef MOTOR_CONTROLLER_LOGGER_DEBUG_LEVEL
#define MOTOR_CONTROLLER_LOGGER_DEBUG_LEVEL LOGGING_DEBUG
#endif

#define MAX_NUMBER_OF_MOTORS 2
#define UPDATE_TIME_MS 100

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct motor_instance_t
{
    struct motor_t motor;
    struct pid_t rpm_pid;
    struct pid_t current_pid;

};

struct motor_controller_t
{
    logging_logger_t *logger_p;
    struct motor_instance_t instances[MAX_NUMBER_OF_MOTORS];
    uint32_t update_time;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct motor_controller_t module;
static struct pid_parameters_t default_parameters =
{
    .kp = 10,
    .ki = 20,
    .kd = 10,
    .imax = 100,
    .cvmax = 100,
    .scale = 100
};

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static inline void InitializeMotors(void);
static inline void UpdateMotors(void);
static inline void UpdateMotorSpeeds(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void MotorController_Init(void)
{
    module = (__typeof__(module)) {0};

    module.logger_p = Logging_GetLogger(MOTOR_CONTROLLER_LOGGER_NAME);
    Logging_SetLevel(module.logger_p, MOTOR_CONTROLLER_LOGGER_DEBUG_LEVEL);

    InitializeMotors();

    Logging_Info(module.logger_p, "MotorController initialized");
}

void MotorController_Update(void)
{
    UpdateMotors();

    if (SysTime_GetDifference(module.update_time) >= UPDATE_TIME_MS)
    {
        UpdateMotorSpeeds();
        module.update_time = SysTime_GetSystemTime();
    }
}

void MotorController_SetRPM(size_t index, int16_t rpm)
{
    assert(index < Board_GetNumberOfMotors());

    PID_SetSetpoint(&module.instances[index].rpm_pid, rpm);
    Logging_Debug(module.logger_p, "M%u sp: {rpm: %i}", index, rpm);
}

void MotorController_SetCurrent(size_t index, int16_t current)
{
    assert(index < Board_GetNumberOfMotors());

    PID_SetSetpoint(&module.instances[index].current_pid, current);
    Logging_Debug(module.logger_p, "M%u sp: {current: %i}", index, current);
}

void MotorController_Coast(size_t index)
{
    assert(index < Board_GetNumberOfMotors());
    assert(0 && "Not implemented!");
}

void MotorController_Brake(size_t index)
{
    assert(index < Board_GetNumberOfMotors());
    assert(0 && "Not implemented!");
}

uint32_t MotorController_GetPosition(size_t index)
{
    assert(index < Board_GetNumberOfMotors());

    return Motor_GetPosition(&module.instances[index].motor);
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static inline void InitializeMotors(void)
{
    const size_t number_of_motors = Board_GetNumberOfMotors();
    assert(number_of_motors <= ElementsIn(module.instances));

    for (size_t i = 0; i < number_of_motors; ++i)
    {
        char name[8];
        snprintf(name, sizeof(name), "M%u", i);
        Motor_Init(&module.instances[i].motor, name, Board_GetMotorConfig(i));

        assert(default_parameters.cvmax >= INT16_MIN && default_parameters.cvmax <= INT16_MAX &&
               "Invalid PID parameter cvmax");
        PID_Init(&module.instances[i].rpm_pid);
        PID_SetParameters(&module.instances[i].rpm_pid, &default_parameters);
        PID_Init(&module.instances[i].current_pid);
        PID_SetParameters(&module.instances[i].current_pid, &default_parameters);
    }
}

static inline void UpdateMotors(void)
{
    const size_t number_of_motors = Board_GetNumberOfMotors();
    for (size_t i = 0; i < number_of_motors; ++i)
    {
        Motor_Update(&module.instances[i].motor);
    }
}

static inline void UpdateMotorSpeeds(void)
{
    const size_t number_of_motors = Board_GetNumberOfMotors();
    for (size_t i = 0; i < number_of_motors; ++i)
    {
        struct motor_instance_t *instance_p = &module.instances[i];
        const int32_t rpm_cv = PID_Update(&instance_p->rpm_pid, Motor_GetRPM(&instance_p->motor));
        const int32_t current_cv = PID_Update(&instance_p->current_pid, Motor_GetCurrent(&instance_p->motor));
        const int32_t cv = (current_cv < rpm_cv) ? current_cv : rpm_cv;
        Motor_SetSpeed(&instance_p->motor, (int16_t)cv);
    }
}
