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

import os
import argparse
from pyclibrary import CParser

__author__ = 'andreas.dahlberg90@gmail.com (Andreas Dahlberg)'
__version__ = '0.1.0'


class Argument(object):
    """Class representing a function argument."""
    _FMT = '{}{}{}{}'

    def __init__(self, c_type, name, pointer, const):
        self.c_type = c_type
        self.name = name
        self.pointer = pointer
        self.const = const

    def to_string(self):
        if self.name:
            name = ' ' + self.name
        else:
            name = ''

        if self.pointer:
            pointer = '*'
        else:
            pointer = ''

        if self.const:
            const = 'const '
        else:
            const = ''

        return Argument._FMT.format(const, self.c_type, pointer, name)


class Function(object):
    """Class representing a C-function."""

    _FMT = '__attribute__((weak)) {} {}({})'

    def __init__(self, c_type, name, arguments):
        self.c_type = c_type
        self.name = name
        self.arguments = arguments

    def to_string(self):

        argument_strings = [x.to_string() for x in self.arguments]

        lines = []
        lines.append(Function._FMT.format(self.c_type, self.name, ', '.join(argument_strings)))
        lines.append('{')
        if self.c_type != 'void':
            lines.append('    mock_type({});'.format(self.c_type))
        lines.append('}')

        return '\r\n'.join(lines)


class HeaderParser(object):
    """Parser"""

    def __init__(self, file_name):
        self.parser = CParser([file_name])

    def get_functions(self):
        """Get a list with all function objects."""

        function_objects = []
        functions = self.parser.defs['functions']

        for func_name in functions:
            args = []

            for arg in functions[func_name][1]:
                arg_name, arg_type, _, = arg

                arg_obj = Argument(
                    arg_type.type_spec,
                    arg_name,
                    is_pointer(arg_type),
                    is_const(arg_type)
                )
                args.append(arg_obj)

            return_type = functions[func_name][0].type_spec
            func_obj = Function(return_type, func_name, args)
            function_objects.append(func_obj)

        return function_objects


def is_pointer(arg_type):
    """Check if the argument type is a pointer."""
    if len(arg_type.declarators) == 1:
        return arg_type.declarators[0] == '*'
    return False


def is_const(arg_type):
    """Check if the argument type is const."""
    if len(arg_type.type_quals[0]) == 1:
        return arg_type.type_quals[0][0] == 'const'
    return False


def parse(args):
    """Parse header file and print the mock functions."""
    header_parser = HeaderParser(args.header)

    functions = header_parser.get_functions()
    c_code = '\r\n\r\n'.join([x.to_string() for x in functions])

    with open(args.template) as template_file:
        template = template_file.read()

        name = os.path.basename(args.header).split('.')[0]
        params = {
            'filename': name,
            'author': 'Andreas Dahlberg',
            'brief': 'Mock functions for {}.'.format(name),
            'header': os.path.basename(args.header),
            'mock_functions': c_code
        }

        print(template.format(**params))


def main():
    """Main function handling command line arguments."""
    parser = argparse.ArgumentParser()
    parser.add_argument('header')
    parser.add_argument('template')

    args = parser.parse_args()
    parse(args)

    return 0


if __name__ == '__main__':
    exit(main())
