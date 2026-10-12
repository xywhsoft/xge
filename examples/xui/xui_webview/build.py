#!/usr/bin/env python3
"""Build the standalone Windows WebView demo using a WebView2-enabled runtime."""
import argparse
import os
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--runtime-dir', type=Path, default=root/'build/webview-basic')
parser.add_argument('--output', type=Path)
args = parser.parse_args()
if sys.platform != 'win32':
    parser.error('The browser backend currently supports Windows only.')
runtime = args.runtime_dir.resolve()
if not all((runtime/p).is_file() for p in ('xge.dll', 'xge.lib', 'WebView2Loader.dll')):
    parser.error('Build the WebView2-enabled normal runtime first; see examples/xui/xui_webview/README.md.')
output = (args.output or runtime/'xui_webview.exe').resolve()
output.parent.mkdir(parents=True, exist_ok=True)
subprocess.run([os.environ.get('CC', 'gcc'), '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                '-DXGE_DLL', '-DXUI_DLL', '@config/includes.rsp', '-o', str(output), 'examples/xui/xui_webview/main.c',
                str(runtime/'xge.lib'), '-lm', '-lws2_32', '-liphlpapi', '-lgdi32', '-luser32',
                '-lshell32', '-lole32', '-lwinmm', '-lavrt'], cwd=root, check=True)
print(output)
