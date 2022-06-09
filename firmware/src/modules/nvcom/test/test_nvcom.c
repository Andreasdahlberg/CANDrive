/**
 * @file   test_nvcom.c
 * @Author Andreas Dahlberg (andreas.dahlberg90@gmail.com)
 * @brief  Test suite for the NVCom module.
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
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdbool.h>
#include "nvcom.h"

//////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VARIABLES
//////////////////////////////////////////////////////////////////////////

static uint16_t fake_backup_registers[20];

//////////////////////////////////////////////////////////////////////////
//LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

static int Setup(void **state)
{
    memset(fake_backup_registers, 0, sizeof(fake_backup_registers));
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//TESTS
//////////////////////////////////////////////////////////////////////////

static void test_NVCom_ColdRestart(void **state)
{
    will_return_always(Board_GetBackupMemoryAddress, (uintptr_t)fake_backup_registers);
    NVCom_Init();

    struct nvcom_data_t *data_p = NVCom_GetData();
    assert_non_null(data_p);
    assert_int_equal(data_p->reset_flags, 0);
    assert_int_equal(data_p->number_of_watchdog_restarts, 0);
    assert_int_equal(data_p->number_of_restarts, 0);
    assert_false(data_p->request_firmware_update);
    assert_false(data_p->firmware_was_updated);
}

static void test_NVCom_WarmRestart(void **state)
{
    will_return_always(Board_GetBackupMemoryAddress, (uintptr_t)fake_backup_registers);
    NVCom_Init();

    struct nvcom_data_t *data_p = NVCom_GetData();
    assert_non_null(data_p);
    data_p->reset_flags = 0x80014002;
    data_p->number_of_watchdog_restarts = 2;
    data_p->number_of_restarts = 3;
    data_p->request_firmware_update = true;
    data_p->firmware_was_updated = true;
    NVCom_SetData(data_p);
    NVCom_Init();

    data_p = NVCom_GetData();
    assert_int_equal(data_p->reset_flags, 0x80014002);
    assert_int_equal(data_p->number_of_watchdog_restarts, 2);
    assert_int_equal(data_p->number_of_restarts, 3);
    assert_true(data_p->request_firmware_update);
    assert_true(data_p->firmware_was_updated);
}

//////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    const struct CMUnitTest test_nvcom[] =
    {
        cmocka_unit_test_setup(test_NVCom_ColdRestart, Setup),
        cmocka_unit_test_setup(test_NVCom_WarmRestart, Setup)
    };

    if (argc >= 2)
    {
        cmocka_set_test_filter(argv[1]);
    }

    return cmocka_run_group_tests(test_nvcom, NULL, NULL);
}
