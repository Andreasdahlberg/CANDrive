/**
 * @file   nvcom.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Non volatile communication module.
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

#ifndef NVCOM_H_
#define NVCOM_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct nvcom_data_t
{
    uint32_t reset_flags;
    uint16_t number_of_watchdog_restarts;
    uint16_t number_of_restarts;
    bool request_firmware_update;
    bool firmware_was_updated;
};

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize the non volatile communication module.
 */
void NVCom_Init(void);

/**
 * Get communication data.
 *
 * @return Pointer to communication data.
 */
struct nvcom_data_t *NVCom_GetData(void);

/**
 * Set communication data.
 *
 * @param data_p Pointer to communication data.
 */
void NVCom_SetData(const struct nvcom_data_t *data_p);

#endif
