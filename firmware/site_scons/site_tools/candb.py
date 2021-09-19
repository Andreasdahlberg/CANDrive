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
from SCons.Script import Dir
import os

__author__ = 'andreas.dahlberg90@gmail.com (Andreas Dahlberg)'

CANTOOLS = 'cantools'

def candb_emitter(target, source, env):
    path_of_calling_sconscript = Dir('.').srcnode().path
    module_name = os.path.basename(path_of_calling_sconscript)
    target.extend([module_name + '.c', module_name + '.h'])
    return target, source


def generate(env):
    """Add Builders and construction variables for the compiler to an Environment."""
    cc.generate(env)

    env['CANTOOLS'] = env.Detect(CANTOOLS)
    env['CANTOOLSCOM'] = '$CANTOOLS generate_c_source --no-floating-point-numbers --database-name ${TARGET.filebase} --output-directory ${TARGET.dir} ${SOURCE}'
    env['CANTOOLSSRCSUFFIX']  = '.dbc'
    env.Append(BUILDERS={
        'CANDB': _get_candb_builder()
    })


def exists(env):
    return env.Detect('CANTOOLS')


def _get_candb_builder():

    return SCons.Builder.Builder(
        action=SCons.Action.Action('$CANTOOLSCOM'),
        emitter=candb_emitter,
        srcsuffix = '$CANTOOLSSRCSUFFIX'
    )
