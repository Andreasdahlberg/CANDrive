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

import threading
import sys
import signal
import argparse
import can
import cantools


class MotorData():
    """Motor data class"""

    def __init__(self):
        self.set_rpm = 0
        self.actual_rpm = 0
        self.set_current = 0
        self.actual_current = 0
        self.status = 0
        self.mode = 0


class Motor():
    """Motor class"""
    def __init__(self, name, data):
        self._name = name
        self._data = data

    @property
    def name(self):
        """Motor name used as identifier"""
        return self._name

    @property
    def rpm(self):
        """Motor speed in revelations per minute(RPM)"""
        return self._data.actual_rpm

    @rpm.setter
    def rpm(self, rpm):
        """Set the desired motor speed in revelations per minute(RPM)"""
        self._data.set_rpm = rpm

    @property
    def current(self):
        """Motor current consumption in mAh"""
        return self._data.actual_current

    @current.setter
    def current(self, current):
        """Set the max motor current in mAh"""
        self._data.set_current = current

    @property
    def mode(self):
        """Get the motor mode"""
        return self._data.mode

    @mode.setter
    def mode(self, mode):
        self._data.mode = mode

    @property
    def status(self):
        """Get the motor status"""
        return self._data.status

    def __str__(self):
        return str(self.__class__) + ": " + str(self.__dict__)


class CanDriveInterface():
    """Interface for controlling CANDrive devices"""
    def __init__(self, channel, dbc_file):
        self._can_bus = can.ThreadSafeBus(channel, bustype='socketcan')
        self._database = cantools.database.load_file(dbc_file)

        self._exit_event = threading.Event()
        signal.signal(signal.SIGINT, self._signal_handler)

        self._receive_thread = threading.Thread(target=self._receive_thread_func)
        self._transmit_thread = threading.Thread(target=self._transmit_thread_func)

        self._motor_data = [MotorData(), MotorData()]
        self._motors = [Motor('M1', self._motor_data[0]), Motor('M2', self._motor_data[1])]

    def _on_motor_msg_status(self, message):
        result = self._database.decode_message(message.arbitration_id, message.data)
        self._motor_data[0].actual_rpm = result['MotorMsgStatusSigRPM1']
        self._motor_data[0].actual_current = result['MotorMsgStatusSigCurrent1']
        self._motor_data[0].status = result['MotorMsgStatusSigStatus1']
        self._motor_data[1].actual_rpm = result['MotorMsgStatusSigRPM2']
        self._motor_data[1].actual_current = result['MotorMsgStatusSigCurrent2']
        self._motor_data[1].status = result['MotorMsgStatusSigStatus2']

    def _signal_handler(self, *_):
        self._exit_event.set()

    def _receive_thread_func(self):
        frame_handlers = {
            self._database.get_message_by_name('MotorMsgStatus').frame_id: self._on_motor_msg_status
        }

        while True:
            message = self._can_bus.recv(timeout=0.1)
            if message and message.arbitration_id in frame_handlers:
                frame_handlers[message.arbitration_id](message)

            if self._exit_event.is_set():
                break

    def _transmit_thread_func(self):
        while True:
            control_message = self._database.get_message_by_name('ControllerMsgMotorControl')

            data = control_message.encode({
                'ControllerMsgMotorControlSigRPM1': self._motor_data[0].set_rpm,
                'ControllerMsgMotorControlSigCurrent1': self._motor_data[0].set_current,
                'ControllerMsgMotorControlSigMode1': self._motor_data[0].mode,
                'ControllerMsgMotorControlSigRPM2': self._motor_data[1].set_rpm,
                'ControllerMsgMotorControlSigCurrent2': self._motor_data[1].set_current,
                'ControllerMsgMotorControlSigMode2': self._motor_data[1].mode})

            message = can.Message(arbitration_id=control_message.frame_id, data=data, is_extended_id=False)
            self._can_bus.send(message)

            if self._exit_event.wait(timeout=0.1):
                break

    def start(self):
        """Start the CANDrive interface"""
        self._receive_thread.start()
        self._transmit_thread.start()

    def wait_for_shutdown(self):
        """Wait for threads to exit and shutdown the interface"""
        self._receive_thread.join()
        self._transmit_thread.join()
        self._can_bus.shutdown()

    def get_motors(self):
        """Get a list with all available motors"""
        return self._motors

    def get_motor_by_name(self, name):
        """Get all motors matching the supplied name"""
        return [motor for motor in self._motors if motor.name == name]


def main():
    """Create and start the CANDrive interface"""

    parser = argparse.ArgumentParser()
    parser.add_argument('interface')
    parser.add_argument('database')
    args = parser.parse_args()

    interface = CanDriveInterface(args.interface, args.database)
    motor = interface.get_motor_by_name('M1')[0]

    motor.rpm = 10
    motor.mode = 'RUN'
    motor.current = 2000
    interface.start()

    interface.wait_for_shutdown()

if __name__ == '__main__':
    sys.exit(main())
