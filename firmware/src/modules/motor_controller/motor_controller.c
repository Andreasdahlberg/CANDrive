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
#include <inttypes.h>
#include "utility.h"
#include "logging.h"
#include "board.h"
#include "config.h"
#include "systime.h"
#include "pid.h"
#include "system_monitor.h"
#include "motor_controller.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define MOTOR_CONTROLLER_LOGGER_NAME "MoCon"
#ifndef MOTOR_CONTROLLER_LOGGER_DEBUG_LEVEL
#define MOTOR_CONTROLLER_LOGGER_DEBUG_LEVEL LOGGING_INFO
#endif

#define MAX_NUMBER_OF_MOTORS 2
#define UPDATE_TIME_MS 10
#define PID_SCALE 10
#define PID_CV_MAX 1000
#define PID_CV_MIN (-PID_CV_MAX)

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
    uint32_t watchdog_handle;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct motor_controller_t module;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static inline void InitializeMotors(void);
static inline void UpdateMotors(void);
static inline void UpdateMotorSpeeds(void);
static void ResetPIDControllers(struct motor_instance_t *instance_p);
static int32_t LimitValue(int32_t value, int32_t min, int32_t max);
static void UpdateCVLimits(struct pid_parameters_t *parameters_p, int32_t sp);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void MotorController_Init(void)
{
    module = (__typeof__(module)) {0};
    module.watchdog_handle = SystemMonitor_GetWatchdogHandle();
    module.logger_p = Logging_GetLogger(MOTOR_CONTROLLER_LOGGER_NAME);
    Logging_SetLevel(module.logger_p, MOTOR_CONTROLLER_LOGGER_DEBUG_LEVEL);

    InitializeMotors();
    Logging_Info(module.logger_p, "MotorController initialized {wdt_handle: %u}",
                 module.watchdog_handle);
}

void MotorController_Update(void)
{
    UpdateMotors();

    if (SysTime_GetDifference(module.update_time) >= UPDATE_TIME_MS)
    {
        UpdateMotorSpeeds();
        SystemMonitor_FeedWatchdog(module.watchdog_handle);
        module.update_time = SysTime_GetSystemTime();
    }
}

void MotorController_SetRPM(size_t index, int16_t rpm)
{
    assert(index < Config_GetNumberOfMotors());

    const int32_t max_rpm = (int32_t)Config_GetNoLoadRpm();
    const int32_t min_rpm = max_rpm * -1;
    const int32_t limited_rpm = LimitValue(rpm, min_rpm, max_rpm);

    struct pid_parameters_t *parameters_p = PID_GetParameters(&module.instances[index].rpm_pid);
    UpdateCVLimits(parameters_p, limited_rpm);

    PID_SetSetpoint(&module.instances[index].rpm_pid, limited_rpm);
    Logging_Debug(module.logger_p, "M%u sp: {rpm: %i}", index, limited_rpm);
}

void MotorController_SetCurrent(size_t index, int16_t current)
{
    assert(index < Config_GetNumberOfMotors());

    const uint32_t max_board_current = Board_GetMaxCurrent();
    const uint32_t stall_current = Config_GetStallCurrent();
    const int32_t max_current = (max_board_current < stall_current) ? (int32_t)max_board_current : (int32_t)stall_current;
    const int32_t min_current = max_current * -1;
    const int32_t limited_current = LimitValue(current, min_current, max_current);

    struct pid_parameters_t *parameters_p = PID_GetParameters(&module.instances[index].current_pid);
    UpdateCVLimits(parameters_p, limited_current);

    PID_SetSetpoint(&module.instances[index].current_pid, limited_current);
    Logging_Debug(module.logger_p, "M%u sp: {current: %i}", index, limited_current);
}

void MotorController_Run(size_t index)
{
    assert(index < Config_GetNumberOfMotors());
    if (Motor_GetStatus(&module.instances[index].motor) != MOTOR_RUN)
    {
        Motor_SetSpeed(&module.instances[index].motor, 0);
    }
}

void MotorController_Coast(size_t index)
{
    assert(index < Config_GetNumberOfMotors());
    if (Motor_GetStatus(&module.instances[index].motor) != MOTOR_COAST)
    {
        Motor_Coast(&module.instances[index].motor);
        ResetPIDControllers(&module.instances[index]);
    }
}

void MotorController_Brake(size_t index)
{
    assert(index < Config_GetNumberOfMotors());
    if (Motor_GetStatus(&module.instances[index].motor) != MOTOR_BRAKE)
    {
        Motor_Brake(&module.instances[index].motor);
        ResetPIDControllers(&module.instances[index]);
    }
}

uint32_t MotorController_GetPosition(size_t index)
{
    assert(index < Config_GetNumberOfMotors());

    return Motor_GetPosition(&module.instances[index].motor);
}

struct motor_controller_motor_status_t MotorController_GetStatus(size_t index)
{
    assert(index < Config_GetNumberOfMotors());

    const struct motor_instance_t *instance_p = &module.instances[index];
    struct motor_controller_motor_status_t status;
    status.rpm.actual = Motor_GetRPM(&instance_p->motor);
    status.rpm.target = (int16_t)PID_GetSetpoint(&instance_p->rpm_pid);
    status.current.actual = Motor_GetCurrent(&instance_p->motor);
    status.current.target = (int16_t)PID_GetSetpoint(&instance_p->current_pid);
    status.status = Motor_GetStatus(&instance_p->motor);

    return status;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static inline void InitializeMotors(void)
{
    const size_t number_of_motors = Config_GetNumberOfMotors();
    assert(number_of_motors <= ElementsIn(module.instances));

    struct pid_parameters_t pid_parameters =
    {
        .kp = (int32_t)Config_GetValue("kp"),
        .ki = (int32_t)Config_GetValue("ki"),
        .kd = (int32_t)Config_GetValue("kd"),
        .imax = (int32_t)Config_GetValue("imax"),
        .imin = (int32_t)Config_GetValue("imin"),
        .cvmax = PID_CV_MAX,
        .cvmin = PID_CV_MIN,
        .scale = PID_SCALE
    };

    for (size_t i = 0; i < number_of_motors; ++i)
    {
        char name[8];
        snprintf(name, sizeof(name), "M%" PRIu32, (uint32_t)i);
        Motor_Init(&module.instances[i].motor, name, Board_GetMotorConfig(i));

        assert(pid_parameters.cvmax >= INT16_MIN && pid_parameters.cvmax <= INT16_MAX &&
               "Invalid PID parameter cvmax");
        PID_Init(&module.instances[i].rpm_pid);
        PID_SetParameters(&module.instances[i].rpm_pid, &pid_parameters);
        PID_Init(&module.instances[i].current_pid);
        PID_SetParameters(&module.instances[i].current_pid, &pid_parameters);
    }
}

static inline void UpdateMotors(void)
{
    const size_t number_of_motors = Config_GetNumberOfMotors();
    for (size_t i = 0; i < number_of_motors; ++i)
    {
        Motor_Update(&module.instances[i].motor);
    }
}

static inline void UpdateMotorSpeeds(void)
{
    const size_t number_of_motors = Config_GetNumberOfMotors();
    for (size_t i = 0; i < number_of_motors; ++i)
    {
        struct motor_instance_t *instance_p = &module.instances[i];

        if (Motor_GetStatus(&instance_p->motor) == MOTOR_RUN)
        {
            const int32_t rpm_cv = PID_Update(&instance_p->rpm_pid, Motor_GetRPM(&instance_p->motor));
            const int32_t current_cv = PID_Update(&instance_p->current_pid, Motor_GetCurrent(&instance_p->motor));
            const int32_t cv = (current_cv < rpm_cv) ? current_cv : rpm_cv;
            Motor_SetSpeed(&instance_p->motor, (int16_t)cv);
        }
    }
}

static void ResetPIDControllers(struct motor_instance_t *instance_p)
{
    PID_Reset(&instance_p->rpm_pid);
    PID_Reset(&instance_p->current_pid);
}

static inline int32_t LimitValue(int32_t value, int32_t min, int32_t max)
{
    int32_t limited_value;

    if (value < min)
    {
        limited_value = min;
    }
    else if (value > max)
    {
        limited_value = max;
    }
    else
    {
        limited_value = value;
    }

    return limited_value;
}

static void UpdateCVLimits(struct pid_parameters_t *parameters_p, int32_t sp)
{
    if (sp > 0)
    {
        parameters_p->cvmax = PID_CV_MAX;
        parameters_p->cvmin = 0;
    }
    else if (sp < 0)
    {
        parameters_p->cvmax = 0;
        parameters_p->cvmin = PID_CV_MIN;
    }
    else
    {
        /* Keep the last cv limits if set point is set to zero. */
    }
}
