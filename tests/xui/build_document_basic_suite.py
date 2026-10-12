#!/usr/bin/env python3
"""Validate the first usable Document version without the exhaustive advanced matrix."""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import time

root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--out-dir', type=Path, help='New directory for logs and actual exit records')
parser.add_argument('--with-webview', action='store_true', help='Also build and validate the Windows basic WebView release')
args = parser.parse_args()
if sys.platform != 'win32':
    parser.error('This entry validates the Windows UI delivery; use tests/xui/build_document_linux_basic_suite.py for the Linux foundation.')
out = args.out_dir or root / 'build' / ('document-basic-' + datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%SZ') + '-' + str(os.getpid()))
out = out.resolve()
if out.exists():
    parser.error('--out-dir must be new so earlier results remain intact')
out.mkdir(parents=True)
sha = lambda p: hashlib.sha256(Path(p).read_bytes()).hexdigest()
report = {'status': 'running', 'source_root': str(root), 'started_at': time.time(), 'checks': [],
    'advanced_matrix_requested': False, 'webview_requested': args.with_webview,
    'physical_ime_verified': False, 'all_platforms_verified': False, 'overall_design_complete': False}

def save():
    (out / 'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')

def check(name, command, timeout=900):
    log = out / (name + '.log')
    print('Checking ' + name, flush=True)
    started = time.time()
    with log.open('xb') as stream:
        try:
            result = subprocess.run(command, cwd=root, stdout=stream, stderr=subprocess.STDOUT, timeout=timeout)
        except subprocess.TimeoutExpired:
            report['checks'].append({'name': name, 'command': command, 'status': 'timed_out', 'log_sha256': sha(log)})
            raise
    report['checks'].append({'name': name, 'command': command, 'actual_exit': result.returncode,
        'seconds': time.time() - started, 'log_sha256': sha(log)})
    save()
    print(name + ' actual EXIT ' + str(result.returncode), flush=True)
    if result.returncode:
        print(log.read_text(encoding='utf-8', errors='replace')[-6000:], flush=True)
        raise RuntimeError(name + ' failed; see ' + str(log))

def bat(name, path, *parameters):
    check(name, ['cmd.exe', '/d', '/c', str(Path(path)), *map(str, parameters)])

save()
try:
    bat('runtime', 'tools\build\ensure_xge_dll.bat')
    runtime = root / 'build/xge.dll'
    runtime_hash = sha(runtime)
    report['runtime_sha256'] = runtime_hash
    bat('core-dll', 'tests/xui/build_document_dll_test.bat')
    corpus = root / 'artifacts/xui-document-rebuild/commonmark-0.31.2.json'
    env_path = os.environ.get('PATH', '')
    os.environ['PATH'] = str(root / 'build') + os.pathsep + env_path
    # The existing corpus driver also checks private source indices. Keep its
    # real kernel build, but leave its separate large-scale benchmarks to the
    # advanced suite. The original default invocation remains unchanged.
    bat('commonmark-gfm', 'tests/xui/build_document_corpus_test.bat', '--corpus-only', corpus)
    bat('renderer', 'tests/xui/build_document_renderer_test.bat')
    bat('editor', 'tests/xui/build_document_editor_test.bat')
    bat('format-commands', 'tests/xui/build_document_format_command_test.bat')
    bat('native-clipboard', 'tests/xui/build_document_native_clipboard_test.bat')
    check('native-ime-bridge', [sys.executable, str(root/'tests/xui/build_document_ime_bridge_test.py'),
                               '--runtime-dir', str(root/'build'), '--out-dir', str(out/'native-ime-bridge')])
    bat('boundary-deletion', 'tests/xui/build_document_boundary_delete_test.bat')
    bat('text-ranges', 'tests/xui/build_document_accessible_text_test.bat')
    bat('message-list', 'tests/xui/build_message_document_test.bat')
    check('native-document-messages', [sys.executable, str(root / 'tests/xui/build_document_messages_test.py'),
                                      '--runtime-dir', str(root / 'build'), '--out-dir', str(out / 'native-messages')])
    check('native-find-replace', [sys.executable, str(root / 'tests/xui/build_document_demo_find_test.py'),
                                 '--runtime-dir', str(root / 'build'), '--out-dir', str(out / 'native-find'), '--with-format'])
    check('native-window-resize', [sys.executable, str(root / 'tests/xui/build_document_demo_resize_test.py'),
                                  '--runtime-dir', str(root / 'build'), '--out-dir', str(out / 'native-resize'), '--with-find'])
    bat('example-build', 'examples/xui/xui_document/build.bat')
    for name, flag in [('native-render', '--verify'), ('native-async', '--verify-async'),
                       ('native-image-placeholder', '--verify-image-placeholder'), ('native-files', '--verify-files')]:
        check(name, [str(root / 'build/xui_document.exe'), flag])
    bat('native-file-and-close-workflows', 'tests/xui/build_document_demo_workflow_test.bat', '--out-dir', out / 'native-workflows')
    check('public-api', ['powershell.exe', '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', 'tests/xui/check_document_release.ps1'])
    if sha(runtime) != runtime_hash:
        raise RuntimeError('Document runtime changed during validation')
    if args.with_webview:
        bat('webview-basic', 'tests/xui/build_webview_basic_test.bat')
    report.update(status='passed', finished_at=time.time(), automatic_baseline_passed=True,
        manual_platform_acceptance_pending=True)
    save()
except BaseException as error:
    report.update(status='failed', finished_at=time.time(), error=str(error))
    save()
    raise
print('Document basic delivery checks passed. Actual logs: ' + str(out), flush=True)
