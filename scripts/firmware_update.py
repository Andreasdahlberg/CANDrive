"""CANDrive firmware updater"""

import time
import argparse
import struct
import io
import math
from enum import IntEnum, unique
import isotp
import bincopy
from can.interfaces.socketcan import SocketcanBus

crc_table = {}

def generate_crc32_table():
    poly = 0x04C11DB7

    global crc_table

    for i in range(256):
        c = i << 24

        for j in range(8):
            c = (c << 1) ^ poly if (c & 0x80000000) else c << 1

        crc_table[i] = c & 0xffffffff


def crc32_stm(bytes_arr):
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


class ISOTPLink():
    def __init__(self, interface, source, destination, error_handler, wftmax=5):
        self._can_bus = SocketcanBus(channel=interface)
        addr = isotp.Address(isotp.AddressingMode.Normal_11bits, rxid=source, txid=destination)
        parameters = {'wftmax': wftmax}
        self._stack = isotp.CanStack(self._can_bus, address=addr, error_handler=error_handler, params=parameters)

    def send(self, data):
        self._stack.send(data)
        while self._stack.transmitting():
            self._stack.process()
            time.sleep(self._stack.sleep_time())

    def receive(self, timeout=2):
        start_time = time.time()
        while time.time() - start_time < timeout:
            if self._stack.available():
                data = self._stack.recv()
                return data

            self._stack.process()
            time.sleep(self._stack.sleep_time())


class Device():
    def __init__(self, interface, source, destination, wftmax=5):
        self._link = ISOTPLink(interface, source, destination, self._error_handler, wftmax=wftmax)
        self._populate_device_information()
        self._send_status = True

    def _error_handler(self, error):
        """ISO-TP error handler"""
        print('IsoTp error: {} - {}'.format(error.__class__.__name__, str(error)))
        self._send_status = False

    def _populate_device_information(self):
        message = Message(MessageType.REQ_FW_INFO)
        self.send(message.dump())
        data = self._link.receive()
        if data:
            msg = FirmwareInformationMessage.from_data(data)
            self.version = msg.version
            self.hardware_revision = msg.hardware_revision
            self.name = msg.name
            self.id = msg.device_id
            self.git_sha = msg.git_sha
        else:
            self.version = None
            self.hardware_revision = None
            self.name = None
            self.id = None
            self.git_sha = None

    def _send_firmware_header(self, data):

        data_length = len(data)
        data_crc = crc32_stm(data)

        data_header = struct.pack('<III', 0, data_length, data_crc)
        message = Message(MessageType.REQ_FW_HEADER, data_header)
        self.send(message.dump())

    def _send_firmware_data(self, data):
        number_of_pages = math.ceil(len(data) / 1024)
        sent_pages = 0

        byte_stream = io.BytesIO(data)
        while True:
            payload = byte_stream.read(1024)
            if not payload:
                break

            message = Message(MessageType.REQ_FW_DATA, payload)
            if not self.send(message.dump()):
                print('Abort firmware upgrade')
                return
            sent_pages += 1
            print('{}/{} pages sent'.format(sent_pages, number_of_pages))
        print('Firmware upgrade done')

    def upgrade(self, upgrade_file):
        with open(upgrade_file, "rb") as f:
            binary_data = f.read()

        self._send_firmware_header(binary_data)
        self._send_firmware_data(binary_data)
        self._populate_device_information()
        self.reset()
        #TODO: Verify correct version

    def reset(self):
        message = Message(MessageType.REQ_RESET)
        self.send(message.dump())

    def send(self, data):
        self._send_status = True
        self._link.send(data)
        return self._send_status


@unique
class MessageType(IntEnum):
    """Message type identifier"""
    REQ_FW_INFO = 0
    REQ_RESET = 1
    REQ_FW_HEADER = 2
    REQ_FW_DATA = 3


class Message():
    def __init__(self, type_id, payload=b''):
        self._type_id = type_id
        self._size = len(payload)
        self._payload_crc = crc32_stm(payload)
        self._payload = payload

    def dump(self):
        header = struct.pack('<III', self._type_id, self._size, self._payload_crc)
        header_crc = crc32_stm(header)
        return struct.pack('<IIII', self._type_id, self._size, self._payload_crc, header_crc) + self._payload


class FirmwareInformationMessage():
    def __init__(self, message_type, version, hardware_revision, name, device_id, git_sha):
        self.message_type = message_type
        self.version = version
        self.hardware_revision = hardware_revision
        self.name = name
        self.device_id = device_id
        self.git_sha = git_sha

    @classmethod
    def from_data(cls, data):
        message_type, version, hardware_revision, name, id1, id2, id3, git_sha = struct.unpack('<I32sI16sIII14s', data)
        device_id = '{:x}{:x}{:x}'.format(id1, id2, id3)
        return cls(message_type, version, hardware_revision, name, device_id, git_sha)


def handle_info(args):
    """Execute the info command."""
    device = Device(args.interface, args.src_id, args.dest_id, args.w)
    print('version={}, hardware_revision={}, name={}, id={}, git_sha={}'.format(
        device.version.decode('utf-8'),
        device.hardware_revision,
        device.name.decode('utf-8'),
        device.id,
        device.git_sha.decode('utf-8')
    ))


def handle_reset(args):
    """Execute the reset command."""
    device = Device(args.interface, args.src_id, args.dest_id)
    device.reset()


def handle_upgrade(args):
    """Execute the upgrade command."""
    device = Device(args.interface, args.src_id, args.dest_id, args.w)
    device.upgrade(args.path)


def main():
    """Main function handling command line arguments."""
    parser = argparse.ArgumentParser()
    parser.add_argument('interface', type=str, help='Set CAN interface, eg. CAN0')
    parser.add_argument('src_id', type=lambda id: int(id, 0), help='Source CAN Id')
    parser.add_argument('dest_id', type=lambda id: int(id, 0), help='Destination CAN Id')
    subparsers = parser.add_subparsers()

    parser_info = subparsers.add_parser('info', help='Print device information')
    parser_info.add_argument('-w', type=int, default=5, help='Max number of wait indications')
    parser_info.set_defaults(func=handle_info)

    parser_info = subparsers.add_parser('reset', help='Reset device')
    parser_info.set_defaults(func=handle_reset)

    parser_upgrade = subparsers.add_parser('upgrade', help='Upgrade device firmware')
    parser_upgrade.add_argument('path', type=str, help='Path to firmware')
    parser_upgrade.add_argument('-w', type=int, default=5, help='Max number of wait indications')
    parser_upgrade.set_defaults(func=handle_upgrade)

    args = parser.parse_args()
    args.func(args)

if __name__ == '__main__':
    generate_crc32_table()
    main()
