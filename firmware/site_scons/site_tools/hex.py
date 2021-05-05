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

__author__ = 'andreas.dahlberg90@gmail.com (Andreas Dahlberg)'

OBJCOPY = 'arm-none-eabi-objcopy'

def generate(env):
    """Add Builders and construction variables for the compiler to an Environment."""
    cc.generate(env)

    env['CC'] = env.Detect('$CC')
    env['OBJCOPY'] = env.Detect(OBJCOPY)
    env['SHCCFLAGS'] = SCons.Util.CLVar('$CFLAGS')

    env.Append(BUILDERS={
        'Hex': _get_hex_builder()
    })


def exists(env):
    return True


def _get_hex_builder():
    return SCons.Builder.Builder(
        action=SCons.Action.Action(
            "${OBJCOPY} -O ihex ${SOURCES} ${TARGET}"
        )
    )
