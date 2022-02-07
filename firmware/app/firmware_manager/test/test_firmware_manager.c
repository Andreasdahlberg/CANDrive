/**
 * @file   test_firmware_manager.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the firmware manager module.
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

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <libopencm3/stm32/flash.h>
#include "utility.h"
#include "logging.h"
#include "board.h"
#include "isotp.h"
#include "protocol.h"
#include "firmware_manager.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

struct firmware_information_t firmware_information = {.version = 2};
static struct logging_logger_t *dummy_logger;
static isotp_status_callback_t rx_cb_fp;
static isotp_status_callback_t tx_cb_fp;

//////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//MOCKS
//////////////////////////////////////////////////////////////////////////

void ISOTP_Bind(struct isotp_ctx_t *ctx_p, void *rx_buffer_p, size_t rx_buffer_size, uint16_t rx_id, uint16_t tx_id, uint8_t separation_time, isotp_status_callback_t rx_callback_fp, isotp_status_callback_t tx_callback_fp)
{
    assert_non_null(ctx_p);
    assert_non_null(rx_buffer_size);

    rx_cb_fp = rx_callback_fp;
    tx_cb_fp = tx_callback_fp;
}

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static int Setup(void **state)
{
    will_return_always(Logging_GetLogger, dummy_logger);
    will_return(Config_GetValue, 0x001);
    will_return(Config_GetValue, 0x002);
    will_return_maybe(Board_GetApplicationAddress, 0x1000);
    will_return_maybe(Board_GetUpgradeMemoryAddress, 0x2000);
    FirmwareManager_Init();
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_FirmwareManager_Init(void **state)
{
    will_return_always(Logging_GetLogger, dummy_logger);
    will_return(Config_GetValue, 0x001);
    will_return(Config_GetValue, 0x002);
    will_return_maybe(Board_GetApplicationAddress, 0x1000);
    will_return_maybe(Board_GetUpgradeMemoryAddress, 0x2000);
    FirmwareManager_Init();
}

static void test_FirmwareManager_Update(void **state)
{
    expect_function_call(ISOTP_Proccess);
    FirmwareManager_Update();
}

static void test_FirmwareManager_GetFirmwareInformation(void **state)
{
    const uint32_t fake_crc = 0xAABBCCDD;
    struct message_header_t message_header = {REQ_FW_INFO, 0, 0, fake_crc};
    will_return(ISOTP_Receive, sizeof(message_header));
    will_return(ISOTP_Receive, &message_header);
    will_return(CRC_Calculate, fake_crc);

    will_return_maybe(Board_GetUpgradeMemoryAddress, 0x2000);
    will_return_maybe(Board_GetHardwareRevision, 1);

    const struct firmware_info_msg_t info =
    {
        .type = REQ_FW_INFO,
        .version = firmware_information.version,
        .hardware_revision = 1,
        .name = "Test",
        .offset = 0x2000,
        .id = {1, 2, 3}
    };

    will_return(ISOTP_Send, true);
    expect_memory(ISOTP_Send, data_p, &info, sizeof(info));

    rx_cb_fp(ISOTP_STATUS_DONE);
    tx_cb_fp(ISOTP_STATUS_DONE);
}

static void test_FirmwareManager_Reset(void **state)
{
    const uint32_t fake_crc = 0xAABBCCDD;
    struct message_header_t message_header = {REQ_RESET, 0, 0, fake_crc};
    will_return(ISOTP_Receive, sizeof(message_header));
    will_return(ISOTP_Receive, &message_header);
    will_return(CRC_Calculate, fake_crc);

    expect_function_call(Board_Reset);
    rx_cb_fp(ISOTP_STATUS_DONE);
}

static void test_FirmwareManager_WaitForRxSpace(void **state)
{
    /* Expect nothing to happen while waiting */
    rx_cb_fp(ISOTP_STATUS_WAITING);

    const uint32_t fake_crc = 0xAABCDEFF;
    struct message_header_t message_header = {REQ_RESET, 0, 0, fake_crc};
    will_return(ISOTP_Receive, sizeof(message_header));
    will_return(ISOTP_Receive, &message_header);
    will_return(CRC_Calculate, fake_crc);

    expect_function_call(Board_Reset);
    rx_cb_fp(ISOTP_STATUS_DONE);
}

static void test_FirmwareManager_HeaderSizeMismatch(void **state)
{
    const uint32_t fake_crc = 0xAABBCCDD;
    struct message_header_t message_header = {REQ_FW_INFO, 0, 0, fake_crc};
    will_return(ISOTP_Receive, sizeof(message_header) - 1);
    will_return(ISOTP_Receive, &message_header);

    /* Discard the message if the header size is incorrect. */
    rx_cb_fp(ISOTP_STATUS_DONE);
}

static void test_FirmwareManager_HeaderCRCMismatch(void **state)
{
    const uint32_t fake_crc = 0xAABBCCDD;
    struct message_header_t message_header = {REQ_FW_INFO, 0, 0, fake_crc};
    will_return(ISOTP_Receive, sizeof(message_header));
    will_return(ISOTP_Receive, &message_header);
    will_return(CRC_Calculate, 0);

    /* Discard the message if the CRC is incorrect. */
    rx_cb_fp(ISOTP_STATUS_DONE);
}

static void test_FirmwareManager_HeaderUnknownType(void **state)
{
    const uint32_t fake_crc = 0xAABBCCDD;
    struct message_header_t message_header = {REQ_END, 0, 0, fake_crc};
    will_return(ISOTP_Receive, sizeof(message_header));
    will_return(ISOTP_Receive, &message_header);
    will_return(CRC_Calculate, fake_crc);

    /* Discard the message if the type is unknown. */
    rx_cb_fp(ISOTP_STATUS_DONE);
}

static void test_FirmwareManager_DownloadFirmware(void **state)
{
    const uint32_t fake_crc = 0xAABBCCDD;

    will_return_always(Flash_ErasePage, true);
    will_return_always(Flash_Write, true);

    /* Firmware header part */
    struct message_header_t message_header = {REQ_FW_HEADER, 0, fake_crc, fake_crc};
    will_return(ISOTP_Receive, sizeof(message_header));
    will_return(ISOTP_Receive, &message_header);
    will_return(CRC_Calculate, fake_crc);
    will_return_maybe(Board_GetUpgradeMemoryAddress, 0x2000);

    struct firmware_image_t image = {1, 2048, fake_crc};
    will_return(ISOTP_Receive, sizeof(image));
    will_return(ISOTP_Receive, &image);
    will_return(CRC_Calculate, fake_crc);

    rx_cb_fp(ISOTP_STATUS_DONE);

    /* Firmware data part */
    message_header = (struct message_header_t) {REQ_FW_DATA, 0, 0, fake_crc};
    will_return(ISOTP_Receive, sizeof(message_header));
    will_return(ISOTP_Receive, &message_header);
    will_return(CRC_Calculate, fake_crc);

    const uint8_t data[128] = {0};
    for (size_t i = 0; i < 2048 / sizeof(data); ++i)
    {
        will_return(ISOTP_Receive, sizeof(data));
        will_return(ISOTP_Receive, data);
    }
    rx_cb_fp(ISOTP_STATUS_DONE);
}

static void test_FirmwareManager_DownloadFirmware_NoFirmwareHeader(void **state)
{
    const uint32_t fake_crc = 0xAABBCCDD;
    struct message_header_t message_header = {REQ_FW_DATA, 0, 0, fake_crc};
    will_return(ISOTP_Receive, sizeof(message_header));
    will_return(ISOTP_Receive, &message_header);
    will_return(CRC_Calculate, fake_crc);

    /* Discard firmware data message if no header has been received. */
    rx_cb_fp(ISOTP_STATUS_DONE);
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_firmware_manager[] =
    {
        cmocka_unit_test(test_FirmwareManager_Init),
        cmocka_unit_test_setup(test_FirmwareManager_Update, Setup),
        cmocka_unit_test_setup(test_FirmwareManager_GetFirmwareInformation, Setup),
        cmocka_unit_test_setup(test_FirmwareManager_Reset, Setup),
        cmocka_unit_test_setup(test_FirmwareManager_WaitForRxSpace, Setup),
        cmocka_unit_test_setup(test_FirmwareManager_HeaderSizeMismatch, Setup),
        cmocka_unit_test_setup(test_FirmwareManager_HeaderCRCMismatch, Setup),
        cmocka_unit_test_setup(test_FirmwareManager_HeaderUnknownType, Setup),
        cmocka_unit_test_setup(test_FirmwareManager_DownloadFirmware, Setup),
        cmocka_unit_test_setup(test_FirmwareManager_DownloadFirmware_NoFirmwareHeader, Setup),
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_firmware_manager, NULL, NULL);
}
