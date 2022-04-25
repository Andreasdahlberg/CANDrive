/**
 * @file   image.c
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

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stddef.h>
#include "logging.h"
#include "crc.h"
#include "image.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define IMAGE_LOGGER_NAME "Image"
#ifndef IMAGE_LOGGER_DEBUG_LEVEL
#define IMAGE_LOGGER_DEBUG_LEVEL LOGGING_DEBUG
#endif

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct module_t
{
    logging_logger_t *logger_p;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct module_t module;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void Image_Init(void)
{
    module = (__typeof__(module)) {0};
    module.logger_p = Logging_GetLogger(IMAGE_LOGGER_NAME);
    Logging_SetLevel(module.logger_p, IMAGE_LOGGER_DEBUG_LEVEL);
}

const struct image_header_t *Image_GetHeader(const uintptr_t *image_p)
{
    const struct image_header_t *header_p = NULL;
    header_p = (const struct image_header_t *)image_p;

    if (header_p->header_magic == IMAGE_HEADER_MAGIC)
    {
        return header_p;
    }
    else
    {
        Logging_Warning(module.logger_p, "Invalid header: {header_magic: 0x%x}", header_p->header_magic);
        return NULL;
    }
}

bool Image_IsValid(const uintptr_t *image_p)
{

    bool status = false;
    const struct image_header_t *header_p = Image_GetHeader(image_p);
    if (header_p != NULL)
    {
        const size_t image_size = header_p->size;
        const uint8_t *image_content_p = ((uint8_t *)image_p) + 12;
        const uint32_t image_crc = CRC_Calculate(image_content_p, image_size);
        Logging_Debug(module.logger_p, "image_size: %u, image_crc: %u", image_size, image_crc);

        if (header_p->crc == image_crc)
        {
            status = true;
        }
        else
        {
            Logging_Warning(module.logger_p,
                            "Invalid CRC: {header_crc: %u, image_crc: %u}",
                            header_p->crc,
                            image_crc);
        }
    }

    return status;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////
