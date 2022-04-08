#!/usr/bin/env python3
# -*- coding: utf-8 -*
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

__author__ = 'andreas.dahlberg90@gmail.com (Andreas Dahlberg)'
__version__ = '0.1.0'


def _generate_crc32_table():
    poly = 0x04C11DB7

    crc_table = {}

    for i in range(256):
        c = i << 24

        for j in range(8):
            c = (c << 1) ^ poly if (c & 0x80000000) else c << 1

        crc_table[i] = c & 0xffffffff

    return crc_table


def crc32_stm(bytes_arr):
    crc_table = _generate_crc32_table()

    length = len(bytes_arr)
    crc = 0xffffffff

    k = 0
    while length >= 4:

        v = ((bytes_arr[k] << 24) & 0xFF000000) | ((bytes_arr[k+1] << 16) & 0xFF0000) | \
        ((bytes_arr[k+2] << 8) & 0xFF00) | (bytes_arr[k+3] & 0xFF)

        crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ v)]
        crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ (v >> 8))]
        crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ (v >> 16))]
        crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ (v >> 24))]

        k += 4
        length -= 4

    if length > 0:
        v = 0

        for i in range(length):
            v |= (bytes_arr[k+i] << 24-i*8)

        if length == 1:

            v &= 0xFF000000

        elif length == 2:
            v &= 0xFFFF0000

        elif length == 3:
            v &= 0xFFFFFF00

        crc = (( crc << 8 ) & 0xffffffff) ^ crc_table[0xFF & ( (crc >> 24) ^ (v ) )];
        crc = (( crc << 8 ) & 0xffffffff) ^ crc_table[0xFF & ( (crc >> 24) ^ (v >> 8) )];
        crc = (( crc << 8 ) & 0xffffffff) ^ crc_table[0xFF & ( (crc >> 24) ^ (v >> 16) )];
        crc = (( crc << 8 ) & 0xffffffff) ^ crc_table[0xFF & ( (crc >> 24) ^ (v >> 24) )];
    return crc
