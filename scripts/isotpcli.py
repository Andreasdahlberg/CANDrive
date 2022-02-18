"""A command line utility to test ISO15765-2/ISO-TP communication."""

import logging
import time
import argparse
import isotp
from can.interfaces.socketcan import SocketcanBus


def error_handler(error):
    """ISO-TP error handler"""
    logging.warning('IsoTp error happened : %s - %s' % (error.__class__.__name__, str(error)))


def handle_send(args):
    """Execute the send command."""
    bus = SocketcanBus(channel=args.interface)
    addr = isotp.Address(isotp.AddressingMode.Normal_11bits, rxid=args.src_id, txid=args.dest_id)

    parameters = {'wftmax': args.w}
    stack = isotp.CanStack(bus, address=addr, error_handler=error_handler, params=parameters)

    payload = [i % 255 for i in range(0, args.length)]
    stack.send(payload)

    while stack.transmitting():
        stack.process()
        time.sleep(stack.sleep_time())
    print('Send done')
    bus.shutdown()


def handle_recv(args):
    """Execute the recv command."""
    bus = SocketcanBus(channel=args.interface)
    addr = isotp.Address(isotp.AddressingMode.Normal_11bits, rxid=args.src_id, txid=args.dest_id)

    parameters = {'stmin': args.m, 'rx_consecutive_frame_timeout': args.t, 'blocksize': args.b}
    stack = isotp.CanStack(bus, address=addr, error_handler=error_handler, params=parameters)

    while True:
        stack.process()
        if stack.available():
            payload = stack.recv()
            print('payload={}'.format(payload))
            print('length={}'.format(len(payload)))
            break
    bus.shutdown()


def main():
    """Main function handling command line arguments."""
    parser = argparse.ArgumentParser()
    parser.add_argument('interface', type=str, help='Set CAN interface, eg. VCAN0')
    parser.add_argument('src_id', type=lambda id: int(id, 0), help='Source CAN Id')
    parser.add_argument('dest_id', type=lambda id: int(id, 0), help='Destination CAN Id')
    subparsers = parser.add_subparsers()

    parser_send = subparsers.add_parser('send', help='Send payload')
    parser_send.add_argument('length', type=int, help='Length of the payload')
    parser_send.add_argument('-w', type=int, default=5, help='Max number of wait indications')
    parser_send.set_defaults(func=handle_send)

    parser_recv = subparsers.add_parser('recv', help='Receive payload')
    parser_recv.add_argument('-t', type=int, default=1000, help='Consecutive frame timeout in ms')
    parser_recv.add_argument('-m', type=int, default=0, help='Minimum separation time between consecutive frames')
    parser_recv.add_argument('-b', type=int, default=0, help='Block size, number of consecutive frames that a sender should send before expecting flow control')
    parser_recv.set_defaults(func=handle_recv)

    args = parser.parse_args()
    args.func(args)


if __name__ == '__main__':
    main()
