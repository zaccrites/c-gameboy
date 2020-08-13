
import sys
import argparse
import subprocess


def run(args):
    serial_log_file = '/tmp/serial_log.bin'  # TODO
    cmd = [
        args.binary_path,
        args.rom_path,
        '--serial-out', serial_log_file,
    ]
    subprocess.check_call(cmd)
    with open(serial_log_file, 'rb') as f:
        raw_output = f.read()

    # TODO: Catch decode error, likely a test failure
    output_text = raw_output.decode('ascii')
    print(output_text)
    print('-----------------------------')
    if 'Failed' in output_text:
        print('Test Failed')
        return 1
    elif 'Passed' in output_text:
        print('Test Passed')
        return 0
    else:
        print('Unknown Result, assuming test failed')
        return 1


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('binary_path')
    parser.add_argument('rom_path')
    args = parser.parse_args()
    return run(args)


if __name__ == '__main__':
    sys.exit(main())
