/**
 * @file   Filter.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Implementation of an exponential moving average(EMA) filter.
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


#ifndef FILTER_H_
#define FILTER_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

/* Calculates fixed point alpha value at compile time. */
#define FILTER_ALPHA(a) ((uint32_t)((a) * UINT32_MAX))

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct filter_t
{
    int32_t value;
    uint32_t alpha;
};

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize the exponential moving average(EMA) filter.
 *
 * @param self_p        Pointer to filter struct.
 * @param initial_value Initial value of the filter, e.g. the first sample.
 * @param alpha         Smoothing factor, a higher value discounts older
 *                      observations faster. Use `FILTER_ALPHA()` to convert
 *                      a floating point value to an integer at compile time.
 */
void Filter_Init(struct filter_t *self_p, uint32_t initial_value, uint32_t alpha);

/**
 * Process the supplied sample.
 *
 * @param self_p Pointer to filter struct.
 * @param sample Sample to process.
 */
void Filter_Process(struct filter_t *self_p, uint32_t sample);

/**
 * Get the filter output value.
 *
 * @param self_p Pointer to filter struct.
 *
 * @return The filter output value.
 */
uint32_t Filter_Output(const struct filter_t *self_p);

/**
 * Check if the supplied filter is initialized.
 *
 * @param self_p Pointer to filter struct.
 *
 * @return True if initialized, otherwise false.
 */
bool Filter_IsInitialized(const struct filter_t *self_p);

#endif
