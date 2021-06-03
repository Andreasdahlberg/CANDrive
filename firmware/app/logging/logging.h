/**
 * @file   logging.h
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

#ifndef LOGGING_H_
#define LOGGING_H_

//////////////////////////////////////////////////////////////////////////
//INCLUDES
//////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stddef.h>

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

#define Logging_Critical(logger, ...)   Logging_Log(logger, LOGGING_CRITICAL, __FILE__, __LINE__, ##__VA_ARGS__)
#define Logging_Error(logger, ...)      Logging_Log(logger, LOGGING_ERROR, __FILE__, __LINE__, ##__VA_ARGS__)
#define Logging_Warning(logger, ...)    Logging_Log(logger, LOGGING_WARNING, __FILE__, __LINE__, ##__VA_ARGS__)
#define Logging_Info(logger, ...)       Logging_Log(logger, LOGGING_INFO, __FILE__, __LINE__, ##__VA_ARGS__)
#define Logging_Debug(logger, ...)      Logging_Log(logger, LOGGING_DEBUG, __FILE__, __LINE__, ##__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

typedef uint32_t (*logging_time_cb_t)(void);

typedef struct logging_logger_t logging_logger_t;

enum logging_level_t
{
    LOGGING_CRITICAL = 50,
    LOGGING_ERROR = 40,
    LOGGING_WARNING = 30,
    LOGGING_INFO = 20,
    LOGGING_DEBUG = 10,
    LOGGING_NOTSET = 0
};

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

/**
 * Initialize the logging module.
 *
 * @param time_callback Callback used to get a timestamp for log entries.
 */
void Logging_Init(logging_time_cb_t time_callback);

/**
 * Get a logger instance.
 *
 * Creates a new instance if the name is unique, otherwise the existing one is returned.
 *
 * @param name_p Name of logger instance.
 *
 * @return Logger instance.
 */
logging_logger_t *Logging_GetLogger(const char *name_p);

/**
 * Set the log level for a logging instance.
 *
 * @param logger Logger instance.
 * @param level  Log level.
 */
void Logging_SetLevel(logging_logger_t *logger_p, enum logging_level_t level);

/**
 * Create a log message.
 *
 * It's recommended to use the predefined log macros instead of this function directly.
 *
 * @param logger    Logger instance.
 * @param level     Log level.
 * @param file_p    File name.
 * @param line      Line number.
 * @param message_p Log message.
 */
void Logging_Log(const logging_logger_t *logger_p, enum logging_level_t level, const char *file_p, uint32_t line, const char *message_p, ...);

#endif
