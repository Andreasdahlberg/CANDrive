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

struct adc_input_t
{
    uint8_t channel;
    volatile uint32_t value;
};

typedef struct adc_input_t adc_input_t;

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize the ADC peripheral.
 */
void ADC_Init(void);

/**
 * Initialize the ADC channel instance.
 *
 * @param self_p Pointer to ADC channel instance.
 * @param channel ADC channel.
 */
void ADC_InitChannel(adc_input_t *self_p, uint8_t channel);

/**
 * Start scanning conversions on the initialized channels.
 */
void ADC_Start(void);

/**
 * Get the voltage on the supplied channel.
 *
 * @param self_p Pointer to ADC channel instance.
 *
 * @return Voltage in mV.
 */
uint32_t ADC_GetVoltage(const adc_input_t *self_p);

#ifdef UNIT_TEST
/**
 * Get the sample buffer.
 *
 * @return Pointer to sample buffer.
 */
volatile uint32_t *ADC_GetSampleBuffer(void);
#endif

#endif
