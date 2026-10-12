#!/usr/bin/env python3
"""Build the native Document/MessageList example against a normal runtime."""
import argparse
import os
from pathlib import Path
import subprocess
import sys


def main():
    root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-dir', type=Path, help='Existing normal XGE build directory')
    parser.add_argument('--output', type=Path, help='Executable path')
    args = parser.parse_args()
    windows = sys.platform == 'win32'
    if not windows and not sys.platform.startswith('linux'):
        parser.error('The current native example builder supports Windows and Linux.')
    runtime = (args.runtime_dir or root / ('build' if windows else 'build/linux-native')).resolve()
    library = runtime / ('xge.lib' if windows else 'libxge.so')
    if not library.is_file():
        parser.error('Build the normal XGE runtime first or provide --runtime-dir.')
    target = (args.output or runtime / ('xui_document_messages.exe' if windows else 'xui_document_messages')).resolve()
    target.parent.mkdir(parents=True, exist_ok=True)
    command = [os.environ.get('CC', 'gcc'), '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
               '-DXGE_DLL', '-DXUI_DLL', '@config/includes.rsp', '-o', str(target), 'examples/xui/xui_document_messages/main.c']
    if windows:
        command += [str(library), '-lm', '-lws2_32', '-liphlpapi', '-lgdi32', '-luser32',
                    '-lshell32', '-lole32', '-lwinmm', '-lavrt']
    else:
        command += ['-L' + str(runtime), '-Wl,-rpath,' + str(runtime), '-lxge', '-lm']
    subprocess.run(command, cwd=root, check=True)
    print('Built native Document messages: ' + str(target), flush=True)


if __name__ == '__main__':
    main()
