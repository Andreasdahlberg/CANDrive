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

#ifndef FIFO_H_
#define FIFO_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define FIFO_New(data) (struct fifo_t){(uint8_t *)data, 0, 0, sizeof(data[0]), (sizeof(data) / sizeof(data[0])), 0}

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct fifo_t
{
    uint8_t *data_p;
    uint8_t head;
    uint8_t tail;
    uint8_t element_size;
    uint8_t max_number_of_elements;
    uint8_t number_of_elements;
};

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

/**
 * Push an item to the FIFO
 *
 * @param fifo Pointer to FIFO.
 * @param item Pointer to item to push.
 *
 * @return Status of push. False if FIFO is full, otherwise true.
 */
bool FIFO_Push(struct fifo_t *self_p, const void *item_p);

/**
 * Get and remove the first item in the FIFO.
 *
 * @param fifo Pointer to FIFO.
 * @param item Pointer to item where the new item will be stored.
 *
 * @return Status of pop. False if FIFO is empty, otherwise true.
 */
bool FIFO_Pop(struct fifo_t *self_p, void *item_p);

/**
 * Get the first item in the FIFO without removing it.
 *
 * @param fifo Pointer to FIFO.
 * @param item Pointer to item where the new item will be stored.
 *
 * @return Status of peek. False if FIFO is empty, otherwise true.
 */
bool FIFO_Peek(const struct fifo_t *self_p, void *item_p);

/**
 * Check if FIFO is full.
 *
 * @param fifo Pointer to FIFO.
 *
 * @return True if FIFO is full, otherwise false.
 */
bool FIFO_IsFull(const struct fifo_t *self_p);

/**
 * Check if FIFO is empty.
 *
 * @param fifo Pointer to FIFO.
 *
 * @return True if FIFO is empty, otherwise false.
 */
bool FIFO_IsEmpty(const struct fifo_t *self_p);

/**
 * Reset FIFO.
 *
 * @param fifo Pointer to FIFO.
 */
void FIFO_Clear(struct fifo_t *self_p);

/**
 * Get the number of available slots in the FIFO.
 *
 * @param fifo Pointer to FIFO.
 *
 * @return Number of available slots.
 */
uint8_t FIFO_GetAvailableSlots(const struct fifo_t *self_p);

#endif
