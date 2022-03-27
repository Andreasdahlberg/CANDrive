/**
 * @file   motor.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Motor module.
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

#ifndef MOTOR_H_
#define MOTOR_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "logging.h"
#include "pwm.h"
#include "adc.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

enum motor_direction_t
{
    MOTOR_DIR_CW = 0,
    MOTOR_DIR_CCW
};

enum motor_status_t
{
    MOTOR_UNKNOWN = 0,
    MOTOR_RUN,
    MOTOR_COAST,
    MOTOR_BRAKE,
    MOTOR_SHORT_TO_GROUND,
    MOTOR_SHORT_TO_VCC,
    MOTOR_OPEN_LOAD,
    MOTOR_THERMAL_SHUTDOWN
};

struct motor_driver_config_t
{
    uint32_t port;
    uint16_t sel;
    uint16_t cs;
    uint16_t ina;
    uint16_t inb;
    enum rcc_periph_clken gpio_clock;
};

struct motor_encoder_config_t
{
    uint32_t port;
    uint16_t a;
    uint16_t b;
    enum rcc_periph_clken gpio_clock;
    uint32_t timer;
    enum rcc_periph_clken timer_clock;
    enum rcc_periph_rst timer_rst;
};

struct motor_adc_config_t
{
    uint8_t channel;
};

struct motor_config_t
{
    struct pwm_config_t pwm;
    struct motor_driver_config_t driver;
    struct motor_encoder_config_t encoder;
    struct motor_adc_config_t adc;
};

struct motor_t
{
    pwm_output_t pwm_output;
    adc_input_t adc_input;
    const struct motor_config_t *config_p;
    logging_logger_t *logger_p;
    int16_t speed;
    enum motor_status_t status;
    int32_t count;
    int32_t counts_per_revolution;
    int16_t rpm;
    uint32_t timer;
};

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize the motor instance.
 *
 * @param self_p Pointer to motor instance.
 * @param name Name of the motor instance, used for logging.
 * @param config_p Pointer to motor configuration struct.
 */
void Motor_Init(struct motor_t *self_p,
                const char *name,
                const struct motor_config_t *config_p);

/**
 * Update the internal state of the motor instance.
 *
 * @param self_p Pointer to motor instance.
 */
void Motor_Update(struct motor_t *self_p);

/**
 * Get the motor RPM.
 *
 * @param self_p Pointer to motor instance.
 *
 * @return Motor RPM.
 */
int16_t Motor_GetRPM(const struct motor_t *self_p);

/**
 * Get the current consumption.
 *
 * @param self_p Pointer to motor instance.
 *
 * @return Motor current in mA.
 */
int16_t Motor_GetCurrent(const struct motor_t *self_p);

/**
 * Set the motor speed.
 *
 * @param self_p Pointer to motor instance.
 * @param speed  Motor speed, direction depends on the speed sign. [-100, 100]
 */
void Motor_SetSpeed(struct motor_t *self_p, int16_t speed);

/**
 * Stop the motor without braking.
 *
 * @param self_p Pointer to motor instance.
 */
void Motor_Coast(struct motor_t *self_p);

/**
 * Stop the motor and brake.
 *
 * @param self_p Pointer to motor instance.
 */
void Motor_Brake(struct motor_t *self_p);

/**
 * Get the motor fault status.
 *
 * @param self_p Pointer to motor instance.
 *
 * @return Fault status.
 */
enum motor_status_t Motor_GetStatus(const struct motor_t *self_p);

/**
 * Get the motor direction.
 *
 * This function returns the measured direction and this can differ from the
 * commanded direction.
 *
 * @param self_p Pointer to motor instance.
 *
 * @return Motor direction.
 */
enum motor_direction_t Motor_GetDirection(const struct motor_t *self_p);

/**
 * Get the current motor position.
 *
 * @param self_p Pointer to motor instance.
 *
 * @return Motor position in degrees.
 */
uint32_t Motor_GetPosition(const struct motor_t *self_p);

const char *Motor_DirectionToString(const struct motor_t *self_p, enum motor_direction_t direction);

#endif
