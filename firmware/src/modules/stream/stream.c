/**
 * @file   stream.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Implementation of a buffered stream.
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

#include <string.h>
#include <assert.h>
#include "stream.h"

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

void Stream_Init(struct stream_t *self_p, void *buffer_p, size_t size)
{
    assert(self_p != NULL);
    assert(buffer_p != NULL);
    assert(size > 0);

    *self_p = (__typeof__(*self_p)) {0};
    self_p->data_p = buffer_p;
    self_p->size = size;
}

size_t Stream_Write(struct stream_t *self_p, const void *source_p, size_t length)
{
    assert(self_p != NULL);
    assert(source_p != NULL);

    const size_t available_space = Stream_GetAvailableSpace(self_p);
    const size_t number_of_bytes = available_space >= length ? length : available_space;

    for (size_t i = 0; i < number_of_bytes; ++i)
    {
        self_p->data_p[self_p->head % self_p->size] = ((const uint8_t *)source_p)[i];
        ++self_p->head;
    }

    self_p->number_of_bytes += number_of_bytes;

    return number_of_bytes;
}

size_t Stream_Read(struct stream_t *self_p, void *destination_p, size_t length)
{
    assert(self_p != NULL);
    assert(destination_p != NULL);

    const size_t number_of_bytes = self_p->number_of_bytes >= length ? length : self_p->number_of_bytes;

    for (size_t i = 0; i < number_of_bytes; ++i)
    {
        ((uint8_t *)destination_p)[i] = self_p->data_p[self_p->tail % self_p->size];
        ++self_p->tail;
    }

    self_p->number_of_bytes -= number_of_bytes;

    return number_of_bytes;
}

size_t Stream_GetAvailableSpace(const struct stream_t *self_p)
{
    assert(self_p != NULL);

    return self_p->size - self_p->number_of_bytes;
}

bool Stream_HasData(const struct stream_t *self_p)
{
    assert(self_p != NULL);

    return self_p->number_of_bytes > 0;
}

void Stream_Clear(struct stream_t *self_p)
{
    assert(self_p != NULL);

    self_p->head = 0;
    self_p->tail = 0;
    self_p->number_of_bytes = 0;
}
