#!/usr/bin/env python3
"""Run the real-window, large-document and first-frame caret acceptance entries."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import sys
import time

tests = Path(__file__).resolve().parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source-root', type=Path, default=tests.parent)
parser.add_argument('--build-dir', type=Path, required=True, help='Existing matching XGE build directory')
parser.add_argument('--out-dir', type=Path, required=True, help='New directory, preserving previous results')
parser.add_argument('--font', type=Path)
parser.add_argument('--headless', action='store_true', help='Linux Xvfb/Mesa software OpenGL')
args = parser.parse_args()
windows = sys.platform == 'win32'
if not windows and sys.platform != 'linux':
    parser.error('The native acceptance entries currently support Windows and Linux')
if windows and args.headless:
    parser.error('--headless is Linux-only')
root, build, out = (p.resolve() for p in (args.source_root, args.build_dir, args.out_dir))
if out.exists():
    parser.error('--out-dir must be new')
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
entries = [
    ('window', 'build_document_native_window_test.py', 4),
    ('scale', 'build_document_native_scale_test.py', 2),
    ('caret', 'build_document_native_caret_test.py', 2),
]
paths = [Path(__file__).resolve(), build / 'xge_build_config.h',
    build / ('xge.dll' if windows else 'libxge.so'),
    build / ('xge.lib' if windows else 'libxge.so')]
paths.extend(tests / name for _, name, _ in entries)
paths.extend(tests / name for name in [
    'xge_gl_system_header_test.c', 'xui_document_native_window_test.c',
    'xui_document_native_scale_test.c', 'xui_document_native_caret_test.c'])
if args.font:
    args.font = args.font.resolve()
    paths.append(args.font)
missing = [str(p) for p in paths if not p.is_file()]
if missing:
    parser.error('Missing inputs: ' + ', '.join(missing))
inputs = {str(p): sha(p) for p in paths}
out.mkdir(parents=True)
report = {'status': 'running', 'started_at': time.time(), 'platform': platform.platform(),
    'machine': platform.node(), 'source_root': str(root), 'build_dir': str(build),
    'input_sha256': inputs, 'headless_software_gl': args.headless,
    'worker_frozen': False, 'entries': [], 'actual_product_checks': 0,
    'physical_ime_dpi_readers_verified': False, 'fixed_machine_release_gate_verified': False}


def save():
    (out / 'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')


save()
try:
    for name, script, count in entries:
        command = [sys.executable, str(tests / script), '--source-root', str(root),
            '--build-dir', str(build), '--out-dir', str(out / name)]
        if args.font:
            command += ['--font', str(args.font)]
        if args.headless:
            command.append('--headless')
        log = out / (name + '.log')
        with log.open('x', encoding='utf-8') as stream:
            result = subprocess.run(command, cwd=root, stdout=stream,
                stderr=subprocess.STDOUT, timeout=900)
        entry = {'name': name, 'command': command, 'actual_exit': result.returncode,
            'log_sha256': sha(log)}
        report['entries'].append(entry)
        save()
        print(name, 'entry actual EXIT', result.returncode, flush=True)
        if result.returncode:
            print(log.read_text(encoding='utf-8', errors='replace')[-7000:], flush=True)
            report['status'] = 'failed'
            save()
            raise SystemExit(result.returncode)
        child_path = out / name / 'report.json'
        child = json.loads(child_path.read_text(encoding='utf-8'))
        if child['status'] != 'passed' or len(child['checks']) != count:
            raise RuntimeError('Incomplete native acceptance entry: ' + name)
        for check in child['checks']:
            if check['actual_exit'] != 0:
                raise RuntimeError('Child product check did not pass: ' + name)
            if sha(out / name / (check['name'] + '.log')) != check['log_sha256']:
                raise RuntimeError('Child log changed: ' + name)
        for value, checksum in child['input_sha256'].items():
            if sha(Path(value)) != checksum:
                raise RuntimeError('Child input changed: ' + value)
        entry['report_sha256'] = sha(child_path)
        entry['actual_product_checks'] = count
        report['actual_product_checks'] += count
        save()
    if report['actual_product_checks'] != 8:
        raise RuntimeError('All eight product compile/run checks are required')
    if not all(sha(Path(p)) == h for p, h in inputs.items()):
        raise RuntimeError('Suite input changed during run')
    report['images'] = []
    for name, _, _ in entries:
        for path in sorted((out / name).glob('*.png')):
            data = path.read_bytes()
            if data[:8] != b'\x89PNG\r\n\x1a\n':
                raise RuntimeError('Invalid native output image: ' + str(path))
            report['images'].append({'entry': name, 'name': path.name,
                'path': str(path), 'sha256': sha(path),
                'width': int.from_bytes(data[16:20], 'big'),
                'height': int.from_bytes(data[20:24], 'big')})
    if len(report['images']) != 16:
        raise RuntimeError('All16 real-window output images are required')
    report['status'] = 'passed'
    report['finished_at'] = time.time()
    save()
except BaseException as error:
    if report['status'] == 'running':
        report['status'] = 'failed'
        report['error'] = str(error)
        save()
    raise
print('Native Document suite: eight product checks and16 real-window images passed; visual review and physical platform acceptance remain.', flush=True)
