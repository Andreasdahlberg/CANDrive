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

import re
from datetime import datetime
import subprocess
import argparse
import sys
import os
import time
import threading
import serial
from prompt_toolkit.patch_stdout import patch_stdout
from prompt_toolkit import PromptSession
from prompt_toolkit.auto_suggest import AutoSuggestFromHistory

__author__ = 'andreas.dahlberg90@gmail.com (Andreas Dahlberg)'
__version__ = '0.1.0'


class Colors:
    """ANSI color definitions"""
    END = '\033[0m'
    BOLD = '\033[1m'
    BLACK = '\033[30m'
    RED = '\033[31m'
    BRIGHT_RED = '\033[31;1m'
    GREEN = '\033[32m'
    YELLOW = '\033[33m'
    BLUE = '\033[34m'
    MAGENTA = '\033[35m'
    CYAN = '\033[36m'
    WHITE = '\033[37m'
    BLINK = '\033[5m'
    DEBUG = GREEN
    INFO = BLUE
    WARNING = YELLOW
    ERROR = RED
    CRITICAL = RED


class LogLevels:
    """Log levels used by CANDrive FW"""
    CRITICAL = 50
    ERROR = 40
    WARNING = 30
    INFO = 20
    DEBUG = 10
    NOTSET = 0


def get_formated_output(values):
    return '{host_timestamp} {timestamp} {level} {module} {file}:{line} {message}'.format(**values)


def addr2line(binary, address):
    """Convert address to file and line number"""
    proc = subprocess.Popen(['addr2line', '-e', binary, address],
                            stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT
                           )
    out, _ = proc.communicate()

    if not out.decode('utf-8').startswith('??'):
        return out.decode('utf-8').strip()

    return None


def add_color_to_level(level):
    """Add ANSI color code to level string"""
    color_code = Colors.GREEN

    if level == "DEBUG":
        color_code = Colors.GREEN
    elif level == "INFO":
        color_code = Colors.INFO
    elif level == "WARNING":
        color_code = Colors.WARNING
    elif level == "ERROR":
        color_code = Colors.ERROR
    elif level == "CRITICAL":
        color_code = Colors.CRITICAL
    return color_code + level + Colors.END


class Monitor():

    def __init__(self, port, baudrate, timeout=0.2, exclude=None):
        self._serial_port = serial.Serial(port, baudrate, timeout=timeout)
        self._log_level = LogLevels.NOTSET
        self._exclude = exclude
        self._elf = None

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        self._serial_port.close()

    @property
    def log_level(self):
        """Get the log level"""
        return self._log_level

    @log_level.setter
    def log_level(self, level):
        """Set the log level"""
        self._log_level = self._log_level_string_to_log_level(level)

    @property
    def elf(self):
        """Get the elf file"""
        return self._elf

    @elf.setter
    def elf(self, elf_file):
        """Set the elf file"""
        self._elf = elf_file

    def read(self):
        """Read serial output and return formatted output"""
        line = self._serial_port.readline()
        values = self._parse_line(line)
        if values and self._filter(values):
            self._add_host_timestamp(values)
            self._modify_values_for_view(values)
            self._add_colors(values)
            return get_formated_output(values)
        return None

    def write(self, data):
        """Write data to the device"""
        for char in data:
            self._serial_port.write(char.encode('utf-8'))
            time.sleep(0.005) #Sleep so that the device can keep up.
        self._serial_port.write('\r'.encode('utf-8'))

    def _parse_line(self, data):
        """Parse line and return values"""
        pattern = r'.*\[(?P<timestamp>\d*)\] (?P<level>\w*):(?P<module>\w*)\s(?P<file>\D*):(?P<line>\d*)\s(?P<message>.*)\r\n'

        if data:
            try:
                line = data.decode('utf-8')
            except UnicodeDecodeError:
                return None

            result = re.match(pattern, line)
            if result:
                values = {
                    'timestamp': result.group('timestamp'),
                    'level': result.group('level'),
                    'module': result.group('module'),
                    'file': result.group('file'),
                    'line': result.group('line'),
                    'message': result.group('message'),
                }
                return values

            if line.startswith('assertion'):
                datetime_obj = datetime.now()
                print('{} {}'.format(
                    Colors.BOLD + datetime_obj.strftime('%H:%M:%S.%f')[:-3] + Colors.END,
                    Colors.RED + line + Colors.END))
            elif line.startswith('[FAIL]'):
                print(Colors.RED + 'Command failed' + Colors.END)
            elif line.startswith('[OK]'):
                print(Colors.GREEN + 'Command successful' + Colors.END)
        return None

    def _add_host_timestamp(self, values):
        """Add host timestamp to the supplied values"""
        if values:
            datetime_obj = datetime.now()
            values['host_timestamp'] = datetime_obj.strftime('%H:%M:%S.%f')[:-3]

    def _modify_values_for_view(self, values):
        if values:
            hex_pattern = r'.*(?P<hex>0x80\w*).*'
            hex_result = re.match(hex_pattern, values['message'])

            if hex_result:
                function_name_and_line = addr2line(self._elf, hex_result.group('hex'))
                if function_name_and_line:
                    function_name_and_line = os.path.relpath(function_name_and_line, os.getcwd())
                    values['message'] = values['message'].replace(hex_result.group('hex'), Colors.MAGENTA + function_name_and_line + Colors.END)

    def _add_colors(self, values):
        """Add ANSI color codes to all values"""
        values['host_timestamp'] = Colors.BOLD +  values['host_timestamp'] + Colors.END
        values['timestamp'] = Colors.BOLD + values['timestamp'] + Colors.END
        values['level'] = add_color_to_level(values['level'])
        values['module'] = Colors.CYAN +  values['module'] + Colors.END
        values['message'] = Colors.WHITE +  values['message'] + Colors.END

    def _log_level_string_to_log_level(self, log_level_string):
        """Convert log level string to integer"""
        return getattr(LogLevels, log_level_string)

    def _filter(self, values):
        """Check if the supplied values match the filter rules"""
        log_level_match = self._log_level_string_to_log_level(values['level']) >= self._log_level

        if self._exclude:
            exclude_match = values['module'] not in self._exclude
        else:
            exclude_match = True

        return log_level_match and exclude_match


def wait_for_commands(monitor):
    """Wait for commands from the user"""
    try:
        session = PromptSession()
        while 1:
            with patch_stdout(raw=True):
                result = session.prompt("CANDrive> ", auto_suggest=AutoSuggestFromHistory())
                monitor.write(result)
    except KeyboardInterrupt:
        pass


def start_monitor(args):
    """Start the monitor and print output"""
    with Monitor(port=args.port, baudrate=args.baudrate, exclude=args.exclude) as monitor:
        monitor.log_level = args.level
        monitor.elf = args.elf

        def read_thread_func():
            while 1:
                output = monitor.read()
                if output:
                    print(output)

        read_thread = threading.Thread(target=read_thread_func)
        read_thread.daemon = True
        read_thread.start()

        wait_for_commands(monitor)
    return 0


def main():
    """Main function handling command line arguments."""
    parser = argparse.ArgumentParser()
    parser.add_argument('port', type=str, help='Set serial port')
    parser.add_argument('elf', type=str, help='Set application ELF used for addr2line')
    parser.add_argument('--baudrate', '-b', type=int, default=921600, help='Set baudrate')
    parser.add_argument('--exclude', '-e', type=str, nargs='+',
                        help='Exclude all output from the listed modules.')
    parser.add_argument('--level', '-l', type=str, default='NOTSET',
                        help='Suppress all output with a log level below set level.')
    args = parser.parse_args()
    return start_monitor(args)


if __name__ == '__main__':
    sys.exit(main())
