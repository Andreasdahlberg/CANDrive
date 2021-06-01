/**
 * @file   logging.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Logging module.
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

#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "utility.h"
#include "logging.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define MAX_NUMBER_OF_LOGGERS 4
#define LOGGER_NAME_MAX_LENGTH 16


//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

struct logging_logger_t
{
    enum logging_level_t level;
    char name[LOGGER_NAME_MAX_LENGTH];
};

struct module_t
{
    struct logging_logger_t loggers[MAX_NUMBER_OF_LOGGERS];
    size_t number_of_loggers;
    logging_time_cb_t get_time_cb;
};

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static struct module_t module;

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

static inline void PrintHeader(enum logging_level_t level, const char *name_p, const char *file_p, uint32_t line);
static logging_logger_t GetLoggerByName(const char *name_p);
static inline const char *LevelToString(enum logging_level_t level);

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

void Logging_Init(logging_time_cb_t time_callback)
{
    assert(time_callback != NULL);

    module = (__typeof__(module)) {.get_time_cb = time_callback};
}

logging_logger_t Logging_GetLogger(const char *name_p)
{
    assert(name_p != NULL);

    struct logging_logger_t *logger_p;
    logger_p = GetLoggerByName(name_p);

    /* Get a new logger instance if the name was not found. */
    if (logger_p == NULL && module.number_of_loggers < MAX_NUMBER_OF_LOGGERS)
    {
        logger_p = &module.loggers[module.number_of_loggers];
        strlcpy(logger_p->name, name_p, sizeof(logger_p->name));

        logger_p->level = LOGGING_NOTSET;

        ++module.number_of_loggers;
    }

    return logger_p;
}

void Logging_SetLevel(logging_logger_t logger_p, enum logging_level_t level)
{
    assert(logger_p != NULL);
    logger_p->level = level;
}

void Logging_Log(const logging_logger_t logger_p, enum logging_level_t level, const char *file_p, uint32_t line, const char *message_p, ...)
{
    assert(logger_p != NULL);
    if (level >= logger_p->level)
    {
        assert(message_p != NULL);

        PrintHeader(level, logger_p->name, file_p, line);

        va_list args;
        va_start(args, message_p);
        vprintf(message_p, args);
        va_end(args);

        printf("\r\n");
    }
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static inline void PrintHeader(enum logging_level_t level, const char *name_p, const char *file_p, uint32_t line)
{
    uint32_t timestamp = 0;
    if (module.get_time_cb != NULL)
    {
        timestamp = module.get_time_cb();
    }

    printf("[%lu] %s:%s %s:%lu ", timestamp, LevelToString(level), name_p, file_p, line);
}

static logging_logger_t GetLoggerByName(const char *name_p)
{
    logging_logger_t logger_p = NULL;

    for (size_t i = 0; i < module.number_of_loggers; ++i)
    {
        if (strncmp(module.loggers[i].name, name_p, ElementsIn(module.loggers[i].name)) == 0)
        {
            logger_p = &module.loggers[i];
            break;
        }
    }

    return logger_p;
}

static inline const char *LevelToString(enum logging_level_t level)
{
    switch (level)
    {
        case LOGGING_CRITICAL:
            return "CRITICAL";;
        case LOGGING_ERROR:
            return "ERROR";
        case LOGGING_WARNING:
            return "WARNING";
        case LOGGING_INFO:
            return "INFO";
        case LOGGING_DEBUG:
            return "DEBUG";
        case LOGGING_NOTSET:
            return "NOTSET";
        default:
            return "UNKNOWN";
    }
}
