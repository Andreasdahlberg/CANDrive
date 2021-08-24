/**
 * @file   board.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Board support module.
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

#ifndef BOARD_H_
#define BOARD_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rcc.h>
#include "pwm.h"
#include "motor.h"
#include <stdint.h>
#include <stddef.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct board_id_t
{
    uint32_t offset_0;
    uint32_t offset_4;
    uint32_t offset_8;
};

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize the board.
 */
void Board_Init(void);

/**
 * Get the hardware revision.
 *
 * @return Hardware revision.
 */
uint32_t Board_GetHardwareRevision(void);

/**
 * Get the software revision.
 *
 * @return software revision.
 */
uint32_t Board_GetSoftwareRevision(void);

/**
 * Get the device id.
 *
 * @return 96-bit device id.
 */
struct board_id_t Board_GetId(void);

const struct motor_config_t *Board_GetMotorConfig(size_t index);

#endif