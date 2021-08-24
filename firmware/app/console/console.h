/**
 * @file   console.h
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Console module.
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

#ifndef CONSOLE_H_
#define CONSOLE_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define Console_GetArgument(argument_p) _Generic((argument_p), \
    int32_t *:Console_GetInt32Argument, \
    bool *:Console_GetBoolArgument, \
    char **:Console_GetStringArgument)(argument_p)

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

typedef void (*console_write_func_t)(const char *str);
typedef size_t (*console_read_func_t)(char *str);
typedef bool (*console_command_func_t)(void);

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

void Console_Init(console_write_func_t write_func, console_read_func_t read_func);

void Console_RegisterCommand(const char *name, console_command_func_t handler);

void Console_Process(void);

bool Console_GetInt32Argument(int32_t *argument_p);

bool Console_GetStringArgument(char **argument_p);

bool Console_GetBoolArgument(bool *argument_p);


#endif
