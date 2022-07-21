/**
 * @file   protocol.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Protocol definitions for the update manager.
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

#ifndef UPDATE_MANAGER_PROTOCOL_H_
#define UPDATE_MANAGER_PROTOCOL_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define FW_CHUNK_SIZE 1024

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct firmware_image_t
{
    uint32_t version;
    uint32_t size;
    uint32_t crc;
};

enum msg_type_t
{
    REQ_FW_INFO = 0,
    REQ_RESET,
    REQ_UPDATE,
    REQ_FW_HEADER,
    REQ_FW_DATA,
    REQ_END,
};

struct message_header_t
{
    uint32_t type;
    uint32_t size;
    uint32_t payload_crc;
    uint32_t header_crc;
};

struct request_firmware_info_msg_t
{
};

struct firmware_info_msg_t
{
    uint32_t type;
    char version[32];
    uint32_t hardware_revision;
    char name[16];
    uint32_t id[3];
    char git_sha[14];
} __attribute__((packed));

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

#endif
