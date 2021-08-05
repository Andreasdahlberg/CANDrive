/**
 * @file   adc.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  ADC module.
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

#ifndef ADC_H_
#define ADC_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stddef.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////
/**
 * Initialize the ADC.
 */
void ADC_Init(void);

/**
 * Start scanning conversions on the selected channels.
 *
 * @param channels_p Array with channel indexes.
 * @param number_of_channels Number of channel indexes.
 */
void ADC_Start(uint8_t *channels_p, size_t number_of_channels);

/**
 * Get the voltage for the selected channel.
 *
 * @param  channel Channel index.
 *
 * @return Voltage in mV.
 */
uint32_t ADC_GetVoltage(size_t channel);

#endif
