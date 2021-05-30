/**
 * @file   fifo.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  FIFO-module.
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
#include "fifo.h"

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

static inline void *GetHeadPosition(const struct fifo_t *self_p);
static inline void *GetTailPosition(const struct fifo_t *self_p);
static inline void MoveHead(struct fifo_t *self_p);
static inline void MoveTail(struct fifo_t *self_p);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

bool FIFO_Push(struct fifo_t *self_p, const void *item_p)
{
    assert(self_p != NULL);
    assert(item_p != NULL);

    if (!FIFO_IsFull(self_p))
    {
        void *position_p = GetHeadPosition(self_p);
        memcpy(position_p, item_p, self_p->element_size);

        MoveHead(self_p);
        return true;
    }

    return false;
}

bool FIFO_Pop(struct fifo_t *self_p, void *item_p)
{
    assert(self_p != NULL);
    assert(item_p != NULL);

    if (!FIFO_IsEmpty(self_p))
    {
        const void *position_p = GetTailPosition(self_p);
        memcpy(item_p, position_p, self_p->element_size);

        MoveTail(self_p);
        return true;
    }

    return false;
}

bool FIFO_Peek(const struct fifo_t *self_p, void *item_p)
{
    assert(self_p != NULL);
    assert(item_p != NULL);

    if (!FIFO_IsEmpty(self_p))
    {
        const void *position_p = GetTailPosition(self_p);
        memcpy(item_p, position_p, self_p->element_size);

        return true;
    }

    return false;
}

bool FIFO_IsEmpty(const struct fifo_t *self_p)
{
    assert(self_p != NULL);

    return self_p->number_of_elements == 0;
}

bool FIFO_IsFull(const struct fifo_t *self_p)
{
    assert(self_p != NULL);

    return self_p->max_number_of_elements == self_p->number_of_elements;
}

void FIFO_Clear(struct fifo_t *self_p)
{
    assert(self_p != NULL);

    self_p->head = 0;
    self_p->tail = 0;
    self_p->number_of_elements = 0;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static inline void *GetHeadPosition(const struct fifo_t *self_p)
{
    return self_p->data_p + ((size_t)self_p->head * (size_t)self_p->element_size);
}

static inline void *GetTailPosition(const struct fifo_t *self_p)
{
    return self_p->data_p + ((size_t)self_p->tail * (size_t)self_p->element_size);
}

static inline void MoveHead(struct fifo_t *self_p)
{
    assert(self_p->number_of_elements < UINT8_MAX);

    self_p->head = (self_p->head + 1) % self_p->max_number_of_elements;
    self_p->number_of_elements += 1;
}

static inline void MoveTail(struct fifo_t *self_p)
{
    assert(self_p->number_of_elements > 0);

    self_p->tail = (self_p->tail + 1) % self_p->max_number_of_elements;
    self_p->number_of_elements -= 1;
}
