/**
 * @file   image.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief Helper functions to get image information.
 */

/*
This file is part of SillyCat firmware.

SillyCat firmware is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SillyCat firmware is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SillyCat firmware.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef IMAGE_H_
#define IMAGE_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <libopencm3/cm3/vector.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define IMAGE_HEADER_MAGIC 0xAABB

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

enum image_type_t
{
    IMAGE_TYPE_CANDRIVE_APP = 0,
    IMAGE_TYPE_CANDRIVE_BOOT
};

struct image_header_t
{
    uint16_t header_magic;
    uint16_t header_version;
    uint32_t crc;
    uint32_t size;
    enum image_type_t image_type;
    char version[32];
    uintptr_t vector_address;
    uint32_t reserved;
    char git_sha[14];
} __attribute__((packed));

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize the image module.
 */
void Image_Init(void);

/**
 * Get image header.
 *
 * @param image_p Pointer to firmware image.
 *
 * @return Pointer to image header or NULL if image/header is invalid.
 */
const struct image_header_t *Image_GetHeader(const uintptr_t *image_p);

/**
 * Check if the supplied image is valid.
 *
 * An image is valid if the size is larger then zero and the CRC is correct.
 *
 * @param image_p Pointer to firmware image.
 *
 * @return True if valid, otherwise false.
 */
bool Image_IsValid(const uintptr_t *image_p);

/**
 * Get string representation for an image type.
 *
 * @param  image_type Image type.
 *
 * @return String representation.
 */
const char *Image_TypeToString(enum image_type_t image_type);

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

#endif
