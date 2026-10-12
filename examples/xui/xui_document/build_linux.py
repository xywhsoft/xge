#!/usr/bin/env python3
"""Build the native Document example on Linux against the normal XGE library."""
import argparse
import os
from pathlib import Path
import subprocess
import sys


def main():
    root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-dir', type=Path, help='Use an existing normal/default Linux runtime')
    parser.add_argument('--output', type=Path, help='Executable path; default build/linux-native/xui_document')
    args = parser.parse_args()
    if not sys.platform.startswith('linux'):
        parser.error('This entry requires Linux. Use examples/xui/xui_document/build.bat on Windows.')
    runtime = (args.runtime_dir or root/'build/linux-native').resolve()
    target = (args.output or root/'build/linux-native/xui_document').resolve()
    if not args.runtime_dir:
        subprocess.run([sys.executable, str(root/'tools/build/build.py'), 'default', str(runtime)], cwd=root, check=True)
    if not (runtime/'libxge.so').is_file() or not (runtime/'xge_build_config.h').is_file():
        parser.error('--runtime-dir must contain libxge.so and xge_build_config.h from the normal builder')
    target.parent.mkdir(parents=True, exist_ok=True)
    (root/'artifacts/xui-document-rebuild').mkdir(parents=True, exist_ok=True)
    subprocess.run([os.environ.get('CC', 'gcc'), '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                    '-DXGE_DLL', '-DXUI_DLL', '@config/includes.rsp', '-o', str(target),
                    'examples/xui/xui_document/main.c', '-L'+str(runtime),
                    '-Wl,-rpath,'+str(runtime), '-lxge', '-lm'], cwd=root, check=True)
    print('Built native Document example: ' + str(target), flush=True)


if __name__ == '__main__':
    main()
