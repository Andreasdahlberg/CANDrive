#!/usr/bin/env python
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

import os
import subprocess

import SCons.Builder
import SCons.Action

__author__ = 'andreas.dahlberg90@gmail.com (Andreas Dahlberg)'
__version__ = '0.1.0'

colors = {}
colors['cyan'] = '\033[36m'
colors['purple'] = '\033[35m'
colors['blue'] = '\033[34m'
colors['yellow'] = '\033[33m'
colors['green'] = '\033[32m'
colors['red'] = '\033[31m'
colors['end'] = '\033[0m'

def get_status_message(code):
    """Get a status message for 'code'."""
    if code == 0:
        return '{}OK{}'.format(colors['green'], colors['end'])
    else:
        return '{}FAIL{}'.format(colors['red'], colors['end'])


def build_function(target, source, env):
    """Executes the supplied tests in 'source' to generate coverage information."""
    for test_runner in source:
        proc = subprocess.Popen(['valgrind', '--error-exitcode=1', '--track-origins=yes', test_runner.abspath],
                        stdout=subprocess.PIPE,
                        stderr=subprocess.STDOUT
                        )
        out, err = proc.communicate()
        code = proc.returncode

        test_message = '{}{}{} ==> {}'.format(
            colors['blue'],
            test_runner,
            colors['end'],
            get_status_message(code)
        )
        print(test_message)

        if code != 0:
            print(out)
            raise SCons.Errors.BuildError(
                errstr="Unit test failed", status=code, exitstatus=2,
                filename=test_runner.abspath, executor=None, action=None, command=None,
                exc_info=(None, None, None))

    return None


def _get_coverage_builder():
    return SCons.Builder.Builder(
        action=SCons.Action.Action(build_function, '${COVERAGE_COMSTR}')
    )


def generate(env):
    """Add builders and construction variables to the environment."""

    env.Append(BUILDERS={
        'Coverage': _get_coverage_builder()
    })


def exists(env):
    return True
