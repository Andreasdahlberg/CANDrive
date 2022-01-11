/**
 * @file   stream.h
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

#ifndef STREAM_H_
#define STREAM_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct stream_t
{
    uint8_t *data_p;
    size_t head;
    size_t tail;
    size_t size;
    size_t number_of_bytes;
};

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize a stream instance.
 *
 * @param self_p Pointer to stream instance.
 * @param buffer_p Pointer to buffer where the stream data will be stored.
 * @param size Max number of bytes in stream(Size of buffer).
 */
void Stream_Init(struct stream_t *self_p, void *buffer_p, size_t size);

/**
 * Write data to stream.
 *
* 'Stream_Write' is non-blocking and returns directly if stream buffer is full.
 *
 * @param self_p Pointer to stream instance.
 * @param source_p Pointer to data to write.
 * @param length Number of bytes to write.
 *
 * @return Number of bytes written. If less than 'length', the stream buffer is full.
 */
size_t Stream_Write(struct stream_t *self_p, const void *source_p, size_t length);

/**
 * Read data from stream.
 *
 * 'Stream_Read' is non-blocking and returns directly if no data is available.
 *
 * @param self_p Pointer to stream instance.
 * @param destination_p Pointer to location where data will be stored.
 * @param length Max number of bytes to read.
 *
 * @return Number of bytes read, 0 means that no data was available.
 */
size_t Stream_Read(struct stream_t *self_p, void *destination_p, size_t length);

/**
 * Get the available space in the stream.
 *
 * @param self_p Pointer to stream instance.
 * @return Available space in bytes.
 */
size_t Stream_GetAvailableSpace(const struct stream_t *self_p);

/**
 * Check if the stream has data to read.
 *
 * @param self_p Pointer to stream instance.
 *
 * @return True if data is available, otherwise false.
 */
bool Stream_HasData(const struct stream_t *self_p);

/**
 * Clear all data in the stream.
 *
 * @param self_p Pointer to stream instance.
 */
void Stream_Clear(struct stream_t *self_p);

#endif
