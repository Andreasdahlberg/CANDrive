# -*- coding: utf-8 -*
#
# This file is part of CANDrive.
#
# CANDrive is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# CANDrive is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with CANDrive.  If not, see <http://www.gnu.org/licenses/>.

import SCons.Util
import SCons.Tool.cc as cc
import struct
import subprocess
from crc import crc32_stm

__author__ = 'andreas.dahlberg90@gmail.com (Andreas Dahlberg)'

OBJCOPY = 'arm-none-eabi-objcopy'


def add_crc_and_size(target):
    """Add CRC and size to target."""
    with open(target, 'r+b') as f:
        data = f.read()

    offset = 12
    image_size = len(data) - offset
    image_crc = crc32_stm(data[offset:])

    crc_size = struct.pack('II', image_crc, image_size)
    with open(target, 'r+b') as f:
        f.seek(4)
        f.write(crc_size)


def build_function(target, source, env):
    """Create a bin file with embedded CRC and size."""
    cmd = [
        OBJCOPY,
        '-O',
        'binary',
        source[0].path,
        target[0].path
    ]
    subprocess.check_call(cmd)
    add_crc_and_size(target[0].path)


def generate(env):
    """Add Builders and construction variables for the compiler to an Environment."""
    cc.generate(env)

    env['OBJCOPY'] = env.Detect(OBJCOPY)
    env['SHCCFLAGS'] = SCons.Util.CLVar('$CFLAGS')

    env.Append(BUILDERS={
        'Bin': _get_bin_builder()
    })


def exists(env):
    return True


def _get_bin_builder():
    return SCons.Builder.Builder(
        action=SCons.Action.Action(build_function)
    )
