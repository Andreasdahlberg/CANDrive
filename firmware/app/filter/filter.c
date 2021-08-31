/**
 * @file   Filter.c
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

#include <assert.h>
#include "filter.h"

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

void Filter_Init(struct filter_t *self_p, uint32_t initial_value, uint32_t alpha)
{
    assert(self_p != NULL);

    self_p->value = initial_value;
    self_p->alpha = alpha;
}

void Filter_Process(struct filter_t *self_p, uint32_t sample)
{
    assert(self_p != NULL);

    /* Use 64-bits to prevent overflow. */
    uint64_t tmp = (uint64_t)self_p->alpha * (uint64_t)sample +
                   (uint64_t)(UINT32_MAX - self_p->alpha) * (uint64_t)self_p->value;

    /* Scale back to 32-bit. */
    self_p->value = (uint32_t)((tmp + UINT32_MAX / 2) / UINT32_MAX);
}

uint32_t Filter_Output(const struct filter_t *self_p)
{
    assert(self_p != NULL);

    return self_p->value;
}

bool Filter_IsInitialized(const struct filter_t *self_p)
{
    assert(self_p != NULL);

    return self_p->alpha != 0;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
