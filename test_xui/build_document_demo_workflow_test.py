#!/usr/bin/env python3
"""Run the demo's real-XGE file and close workflows with synthetic XUI input."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time
import uuid

root = Path(__file__).resolve().parent.parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--out-dir', type=Path)
parser.add_argument('--build-dir', type=Path, default=root / 'build')
args = parser.parse_args()
if sys.platform != 'win32':
    parser.error('This native demo entry currently requires Windows and its system fonts.')
out = (args.out_dir or root / 'build' / ('document-workflow-' + str(uuid.uuid4()))).resolve()
build = args.build_dir.resolve()
if out.exists():
    parser.error('--out-dir must be new')
paths = [Path(__file__).resolve(), root / 'test_xui/xui_document_demo_workflow_test.c',
    root / 'test_xui/xui_document_demo_native_close_test.c',
    root / 'examples/xui_document/main.c', root / 'xui.h', root / 'xui_document.h',
    root / 'xui_document_ui.h', root / 'xge.h', build / 'xge.dll', build / 'xge.lib', build / 'xge_build_config.h']
sha = lambda p: hashlib.sha256(Path(p).read_bytes()).hexdigest()
missing = [str(p) for p in paths if not p.is_file()]
if missing:
    parser.error('Missing inputs: ' + ', '.join(missing))
inputs = {str(p): sha(p) for p in paths}
out.mkdir(parents=True)
exe = out / 'document-workflow.exe'
env = dict(os.environ)
env['PATH'] = str(build) + os.pathsep + env.get('PATH', '')
report = {'status': 'running', 'started_at': time.time(), 'input_sha256': inputs,
    'checks': [], 'synthetic_pointer_key_input': True, 'physical_ime_verified': False,
    'full_design_complete': False}

def save():
    (out / 'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')

def check(name, command):
    log = out / (name + '.log')
    with log.open('xb') as stream:
        try:
            result = subprocess.run(command, cwd=root, env=env, stdout=stream, stderr=subprocess.STDOUT, timeout=180)
        except subprocess.TimeoutExpired:
            report['checks'].append({'name': name, 'command': command, 'status': 'timed_out', 'log_sha256': sha(log)})
            save()
            raise
    report['checks'].append({'name': name, 'command': command, 'actual_exit': result.returncode, 'log_sha256': sha(log)})
    save()
    print(name, 'actual EXIT', result.returncode, flush=True)
    if result.returncode:
        print(log.read_text(encoding='utf-8', errors='replace')[-4000:], flush=True)
        raise RuntimeError(name + ' failed')

save()
try:
    cc = os.environ.get('CC', 'gcc')
    check('compile', [cc, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror', '-DXGE_DLL', '-DXUI_DLL', '-I.',
        '-o', str(exe), 'test_xui/xui_document_demo_workflow_test.c', str(build / 'xge.lib'),
        '-lm', '-lws2_32', '-liphlpapi', '-lgdi32', '-luser32', '-lshell32', '-lole32', '-lwinmm', '-lavrt'])
    for mode in ('source', 'visual', 'live'):
        for action in ('cancel', 'save', 'discard', 'clean', 'pending'):
            name = mode + '-' + action
            directory = out / name; directory.mkdir()
            check(name, [str(exe), str(directory), mode, action])
    native_exe = out / 'document-native-close.exe'
    check('compile-native-close', [cc, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror', '-DXGE_DLL', '-DXUI_DLL', '-I.',
        '-o', str(native_exe), 'test_xui/xui_document_demo_native_close_test.c', str(build / 'xge.lib'),
        '-lm', '-lws2_32', '-liphlpapi', '-lgdi32', '-luser32', '-lshell32', '-lole32', '-lwinmm', '-lavrt'])
    native_directory = out / 'native-close'; native_directory.mkdir()
    check('native-close', [str(native_exe), str(native_directory)])
    if not all(sha(p) == h for p,h in inputs.items()):
        raise RuntimeError('Inputs changed during verification')
    report['images'] = [{'path': str(p), 'sha256': sha(p)} for p in sorted(out.glob('*/*.png'))]
    report.update(status='passed', finished_at=time.time(), file_and_close_workflows_passed=True,
        native_windows_char_and_close_passed=True, native_window_flags='XGE_INIT_VSYNC')
    save()
except BaseException as error:
    report.update(status='failed', finished_at=time.time(), error=str(error)); save(); raise
print('Native demo workflows: three Markdown modes, Rich file flow, five close outcomes and queued Windows Unicode/close passed.', flush=True)
