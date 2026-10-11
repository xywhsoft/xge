#!/usr/bin/env python3
"""Verify native Windows clipboard success on a fresh private window station.

Exit 77 means isolation unavailable. No interactive clipboard payload is read
or written; insufficient privileges never fall back to an existing station.
"""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

root = Path(__file__).resolve().parent.parent


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-dir', type=Path, default=root/'build')
    parser.add_argument('--out-dir', type=Path)
    args = parser.parse_args()
    if sys.platform != 'win32':
        parser.error('This entry requires Windows.')
    runtime = args.runtime_dir.resolve()
    required = [runtime/p for p in ('xge.dll', 'xge.lib', 'build-report.json')]
    if not all(p.is_file() for p in required):
        parser.error('Build the normal runtime first.')
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%SZ')
    out = (args.out_dir or root/'build'/('document-windows-clipboard-'+stamp+'-'+str(os.getpid()))).resolve()
    if out.exists():
        parser.error('--out-dir must be new.')
    out.mkdir(parents=True)
    sha = lambda p: hashlib.sha256(Path(p).read_bytes()).hexdigest()
    sources = [Path(__file__).resolve(), root/'test_xui/xui_document_windows_clipboard_test.c',
               root/'test_xui/xui_test_proxy.c', root/'test_xui/xui_test_proxy.h',
               root/'xge.h', root/'xui.h', root/'xui_document.h', root/'xui_document_ui.h']
    files = sources + required
    report = dict(status='running', input_sha256={str(p): sha(p) for p in files}, checks=[],
                  native_success_verified=False, interactive_clipboard_payload_accessed=False,
                  physical_ime_verified=False, rendering_uses_deterministic_proxy=True,
                  overall_goal_complete=False)

    def save():
        (out/'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')

    try:
        shutil.copy2(runtime/'xge.dll', out/'xge.dll')
        if (runtime/'WebView2Loader.dll').is_file():
            shutil.copy2(runtime/'WebView2Loader.dll', out/'WebView2Loader.dll')
        shutil.copy2(root/'test_xui/xui_document_windows_clipboard_test.c', out/'test.c')
        executable = out/'document-windows-clipboard.exe'
        build = [os.environ.get('CC', 'gcc'), '-std=c11', '-O2', '-g', '-Wall', '-Wextra', '-Werror',
                 '-DXGE_DLL', '-DXUI_DLL', '-I.', 'test_xui/xui_document_windows_clipboard_test.c',
                 'test_xui/xui_test_proxy.c', str(runtime/'xge.lib'), '-lm', '-luser32', '-ladvapi32',
                 '-o', str(executable)]
        commands = [('build', build),
                    ('reject-interactive-read', [str(executable), '--peer-read', 'WinSta0']),
                    ('reject-interactive-write', [str(executable), '--peer-write', 'WinSta0']),
                    ('run', [str(executable)])]
        for name, command in commands:
            row = dict(name=name, command=command, log=str(out/(name+'.log')))
            report['checks'].append(row)
            save()
            with Path(row['log']).open('xb') as stream:
                result = subprocess.run(command, cwd=root, stdout=stream,
                                        stderr=subprocess.STDOUT, timeout=180 if name=='build' else 60)
            row.update(actual_exit=result.returncode, log_sha256=sha(row['log']))
            print(name+' actual EXIT '+str(result.returncode), flush=True)
            if name=='build':
                if result.returncode:
                    raise RuntimeError(Path(row['log']).read_text(errors='replace')[-4000:])
                continue
            text = Path(row['log']).read_text(encoding='utf-8', errors='replace')
            if name.startswith('reject-interactive-'):
                row['expected_exit'] = 1
                if result.returncode!=1 or 'private_station_is(station)' not in text or 'clipboard_open_attempted=0; payload_attempted=0.' not in text:
                    raise RuntimeError('Interactive station was not rejected before clipboard access: '+text[-4000:])
                row['rejected_before_clipboard_access'] = True
                continue
            records = [json.loads(line) for line in text.splitlines() if line.startswith('{')]
            if len(records)!=1:
                raise RuntimeError('Missing native isolation result: '+text[-4000:])
            native = records[0]
            report['native_result'] = native
            if native['interactive_clipboard_payload_accessed'] or not native['interactive_station_restored']:
                raise RuntimeError('Native isolation invariant failed.')
            if native['interactive_sequence_before']!=native['interactive_sequence_after']:
                raise RuntimeError('Interactive clipboard sequence changed during the test.')
            if result.returncode==77 and native['status']=='isolation_unavailable':
                if native['private_station_created'] or native['clipboard_payload_operation_attempted']:
                    raise RuntimeError('Unavailable path must not access clipboard payloads.')
                report['status'] = 'unavailable'
                break
            if result.returncode or native['status']!='passed':
                raise RuntimeError(text[-4000:])
            if not native['private_station_created'] or not native['private_station_verified'] or native['successful_cases']!=12 or native['peer_processes']!=12:
                raise RuntimeError('Incomplete native success verification.')
            report.update(status='passed', native_success_verified=True)
        report['interactive_station_guards_verified'] = all(
            row.get('rejected_before_clipboard_access') for row in report['checks']
            if row['name'].startswith('reject-interactive-'))
        report['executable_sha256'] = sha(executable)
        report['private_runtime_sha256'] = sha(out/'xge.dll')
    except BaseException as error:
        report.update(status='failed', error=str(error))
        raise
    finally:
        report['input_changes'] = [str(p) for p in files if sha(p)!=report['input_sha256'][str(p)]]
        if report['input_changes']:
            report['status'] = 'failed'
        save()
    if report['input_changes']:
        raise RuntimeError('Inputs changed during validation.')
    if report['status']=='unavailable':
        print('Native clipboard isolation unavailable; no success-path acceptance.', flush=True)
        return 77
    print('Native clipboard success: 4 modes, 12 cases, 12 isolated peer processes.', flush=True)
    return 0


if __name__ == '__main__':
    sys.exit(main())
