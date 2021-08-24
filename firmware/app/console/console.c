/**
 * @file   console.c
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

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "utility.h"
#include "console.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define CONSOLE_START "> "
#define CONSOLE_DELIMITER " "
#define CONSOLE_MAX_LINE_LENGTH 32
#define CONSOLE_MAX_COMMAND_LENGTH 32
#define CONSOLE_MAX_NUMBER_OF_COMMANDS 4
#define CARRIAGE_RETURN 0x0D
#define BACKSPACE 0x08

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct console_command_t
{
    char name[CONSOLE_MAX_COMMAND_LENGTH];
    console_command_func_t handler;
};

struct console_t
{
    console_write_func_t write;
    console_read_func_t read;
    char line_buffer[CONSOLE_MAX_LINE_LENGTH];
    struct console_command_t commands[CONSOLE_MAX_NUMBER_OF_COMMANDS];
    size_t number_of_commands;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

struct console_t self;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static inline void ClearLineBuffer(void);
static bool ParseLineBuffer(void);
static bool HandleCommand(const char *command);
static void PrintStatus(bool status);
static void HandleCarriageReturn(void);
static void HandleBackspace(void);
static void HandleInput(char c);
static bool IsPrintable(char c);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void Console_Init(console_write_func_t write_func, console_read_func_t read_func)
{
    assert(write_func != NULL);
    assert(read_func != NULL);

    self = (__typeof__(self)) {.write = write_func, .read = read_func};
    ClearLineBuffer();

    self.write("\r\n" CONSOLE_START);
}

void Console_RegisterCommand(const char *name, console_command_func_t handler)
{
    assert(self.number_of_commands < ElementsIn(self.commands));
    assert(name != NULL);
    assert(handler != NULL);

    CopyString(self.commands[self.number_of_commands].name,
               name,
               sizeof(self.commands[self.number_of_commands].name));
    self.commands[self.number_of_commands].handler = handler;
    ++self.number_of_commands;
}

void Console_Process(void)
{
    char c;
    if (self.read(&c) > 0)
    {
        switch (c)
        {
            case CARRIAGE_RETURN:
                HandleCarriageReturn();
                break;

            case BACKSPACE:
                HandleBackspace();
                break;

            default:
                HandleInput(c);
                break;
        }
    }
}

bool Console_GetInt32Argument(int32_t *argument_p)
{
    char *argument_token;
    argument_token = strtok(NULL, CONSOLE_DELIMITER);

    bool status = false;
    if (argument_token != NULL)
    {
        errno = 0;

        char *end_p = NULL;
        long int result = strtol(argument_token, &end_p, 0);

        if (argument_token != end_p || errno == 0)
        {
            *argument_p = (int32_t)result;
            status = true;
        }
    }
    return status;
}

bool Console_GetStringArgument(char **argument_p)
{
    char *argument_token;
    argument_token = strtok(NULL, CONSOLE_DELIMITER);

    bool status = false;
    if (argument_token != NULL)
    {
        *argument_p = argument_token;
        status = true;
    }
    return status;
}

bool Console_GetBoolArgument(bool *argument_p)
{
    bool status = false;

    int32_t argument;
    if (Console_GetInt32Argument(&argument))
    {
        *argument_p = (bool)argument;
        status = true;
    }

    return status;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static inline void ClearLineBuffer(void)
{
    memset(self.line_buffer, '\0', sizeof(self.line_buffer));
}

static bool ParseLineBuffer(void)
{
    char *command_token;
    command_token = strtok(self.line_buffer, CONSOLE_DELIMITER);

    bool status = false;
    if (command_token != NULL)
    {
        status = HandleCommand(command_token);
    }

    return status;
}

static bool HandleCommand(const char *command)
{
    bool status = false;

    for (size_t i = 0; i < self.number_of_commands; ++i)
    {
        if (strcmp(self.commands[i].name, command) == 0)
        {
            status = self.commands[i].handler();
        }
    }

    return status;
}

static void PrintStatus(bool status)
{
    if (status)
    {
        self.write("\r\n[OK]");
    }
    else
    {
        self.write("\r\n[FAIL]");
    }
}

static void HandleCarriageReturn(void)
{
    if (strlen(self.line_buffer) > 0)
    {
        const bool status = ParseLineBuffer();
        PrintStatus(status);
        ClearLineBuffer();
    }
    self.write("\r\n" CONSOLE_START);
}

static void HandleBackspace(void)
{
    const size_t line_length = strlen(self.line_buffer);
    if (line_length > 0)
    {
        /* Clear last char */
        self.line_buffer[line_length - 1] = ' ';
        self.write("\r" CONSOLE_START);
        self.write(self.line_buffer);

        /* Move cursor */
        self.line_buffer[line_length - 1] = '\0';
        self.write("\r" CONSOLE_START);
        self.write(self.line_buffer);
    }
}

static void HandleInput(char c)
{
    if (IsPrintable(c))
    {
        const size_t line_length = strlen(self.line_buffer);
        /* Always keep at least one NULL at the end of the line buffer. */
        if (line_length < sizeof(self.line_buffer) - 1)
        {
            self.line_buffer[line_length] = c;
        }
        self.write("\r" CONSOLE_START);
        self.write(self.line_buffer);
    }
}

static bool IsPrintable(char c)
{
    return c >= 0x20 && c <= 0x7e;
}
