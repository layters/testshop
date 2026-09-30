#!/usr/bin/env python3
"""Linux loopback regressions; needs g++ and OpenSSL development headers.

Run: python3 tests/sam_client_regression.py
SAM's fixed TCP port 7656 must be free. No live router or I2P network is used.
An optional first argument selects another sam_client.cpp for baseline testing.
"""
import base64
import hashlib
import os
from pathlib import Path
import socket
import subprocess
import sys
import tempfile
import threading

ROOT = Path(__file__).resolve().parents[1]


def encode(data):
    return base64.b64encode(data).decode().replace('+', '-').replace('/', '~')


def run_case(executable, mode, destination, private):
    commands = []
    errors = []
    with socket.socket() as listener, tempfile.TemporaryDirectory() as directory:
        listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        listener.bind(('127.0.0.1', 7656))
        listener.listen(1)
        listener.settimeout(5)

        def bridge():
            try:
                with listener.accept()[0] as connection:
                    connection.settimeout(5)
                    with connection.makefile('rwb', buffering=0) as stream:
                        while line := stream.readline():
                            command = line.decode().strip()
                            commands.append(command)
                            if command.startswith('HELLO VERSION '):
                                reply = 'HELLO REPLY RESULT=OK VERSION=3.1'
                            elif command == 'DEST GENERATE SIGNATURE_TYPE=7':
                                reply = f'DEST REPLY PUB={destination} PRIV={private}'
                            elif command.startswith('SESSION CREATE '):
                                assert f'DESTINATION={private} ' in command
                                reply = f'SESSION STATUS RESULT=OK DESTINATION={private}'
                            elif command.startswith('NAMING LOOKUP NAME='):
                                # Strict naming service: private keys are not host names.
                                reply = 'NAMING REPLY RESULT=KEY_NOT_FOUND NAME=invalid'
                            else:
                                raise AssertionError('unexpected command: ' + command)
                            stream.write((reply + '\n').encode())
            except Exception as error:
                errors.append(str(error))

        folder = Path(directory)
        if mode != 'fresh' and mode != 'idle':
            (folder / 'i2p.key').write_text(private)
        (folder / 'expected.pub').write_text(destination)
        if destination:
            decoded = base64.b64decode(destination.replace('-', '+').replace('~', '/'))
            address = base64.b32encode(hashlib.sha256(decoded).digest()).decode().lower().rstrip('=')
            (folder / 'expected.address').write_text(address + '.b32.i2p')
        worker = threading.Thread(target=bridge)
        worker.start()
        result = subprocess.run([str(executable), mode], cwd=directory,
                                capture_output=True, timeout=10)
        worker.join(timeout=6)
        assert not worker.is_alive(), 'mock bridge did not stop'
        assert not errors, errors
        assert result.returncode == 0, result.stderr.decode(errors='replace')
        assert not any(c.startswith('NAMING LOOKUP ') for c in commands), commands
        if mode in ('fresh', 'restore'):
            assert sum(c.startswith('SESSION CREATE ') for c in commands) == 1
            assert (folder / 'i2p.key').read_text() == private
        if mode == 'fresh':
            assert 'DEST GENERATE SIGNATURE_TYPE=7' in commands
        else:
            assert not any(c.startswith('DEST GENERATE ') for c in commands)


def main():
    source = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else ROOT / 'src/core/network/sam_client.cpp'
    with tempfile.TemporaryDirectory() as build:
        executable = Path(build) / 'sam-regression'
        subprocess.run([os.environ.get('CXX', 'g++'), '-std=c++17', '-I',
                        str(ROOT / 'src/core/network'), str(source),
                        str(ROOT / 'tests/sam_client_regression.cpp'), '-lcrypto',
                        '-o', str(executable)], check=True)
        # Synthetic serialized fixtures, never live identities. Include padding,
        # non-default certificate lengths, and destinations exceeding 1024 bytes.
        cases = []
        for certificate in (b'', b'\x00\x07\x00\x00', bytes(range(256)) * 3):
            public = bytes(range(256)) + bytes(range(128))
            public += bytes([5 if certificate else 0]) + len(certificate).to_bytes(2, 'big') + certificate
            destination = encode(public)
            private = encode(public + bytes(256 + 32))
            cases.append(('restore', destination, private))
        cases.append(('fresh', cases[1][1], cases[1][2]))
        cases.append(('idle', '', ''))
        cases.append(('invalid', '', encode(bytes(386))))
        cases.append(('invalid', '', encode(bytes(384) + b'\x05\xff\xff' + bytes(20))))
        failed = 0
        for index, case in enumerate(cases):
            try:
                run_case(executable, *case)
                print(f'PASS {index + 1}: {case[0]}')
            except Exception as error:
                failed += 1
                print(f'FAIL {index + 1}: {case[0]}: {error}')
        return bool(failed)


if __name__ == '__main__':
    sys.exit(main())
