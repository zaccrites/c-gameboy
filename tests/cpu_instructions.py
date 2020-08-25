"""Run automated blargg CPU tests."""

import sys
import argparse
import time
import signal
import subprocess
from datetime import datetime, timedelta


def run(args):
    serial_log_file = '/tmp/serial_log.bin'  # TODO
    cmd = [
        args.binary_path,
        args.rom_path,
        '--serial-out', serial_log_file,
        '--headless',
    ]

    # TODO: Test against stdout/stderr
    start = datetime.now()
    p = subprocess.Popen(cmd)
    while True:
        runtime = datetime.now() - start
        if runtime > timedelta(seconds=30):
            print('Test Timed Out')
            p.terminate()
            return 1

        time.sleep(0.250)
        status = p.poll()
        if status is not None and status != 0:
            print('Emulator exited with status', status)
            return 1

        with open(serial_log_file, 'rb') as f:
            raw_output = f.read()
        try:
            output_text = raw_output.decode('ascii')
        except UnicodeDecodeError:
            print('Failed to decode serial log file contents as ASCII:', repr(raw_output))
            return 1

        if 'Failed' in output_text:
            passed = False
        elif 'Passed' in output_text:
            passed = True
        else:
            if status is not None:
                print('Emulator exited without writing test result to the serial port')
                return 1
            passed = None

        if passed is not None:
            print('-----------------------------')
            print(output_text)
            print('-----------------------------')
            print('Test', 'Passed' if passed else 'Failed')
            p.send_signal(signal.SIGINT)
            return 0 if passed else 1


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('binary_path')
    parser.add_argument('rom_path')
    args = parser.parse_args()
    return run(args)


if __name__ == '__main__':
    sys.exit(main())
