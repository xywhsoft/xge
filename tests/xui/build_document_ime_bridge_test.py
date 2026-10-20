#!/usr/bin/env python3
"""Exercise Windows TSF -> XGE queue -> DocumentEditor using current production objects."""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-dir', type=Path, default=root/'build', help='Normal Windows build including current obj dependency records')
    parser.add_argument('--out-dir', type=Path, help='New directory; previous failures are preserved')
    args = parser.parse_args()
    if sys.platform != 'win32':
        parser.error('This entry drives the Windows TSF backend; it is not physical IME acceptance.')
    runtime = args.runtime_dir.resolve()
    build = json.loads((runtime/'build-report.json').read_text(encoding='utf-8'))
    if '-DXUI_ENABLE_WEBVIEW2' in build['flags']:
        parser.error('Use the normal runtime; the standalone WebView build has a different link dependency set.')
    objects = [runtime/'obj'/(p.replace('/', '_')+'.o') for p in build['sources'] if p != 'single/xge.c']
    if 'XGE_ENABLE_HARFBUZZ' in build['features']:
        objects.append(root/'build/shared/obj/extlibs_harfbuzz_src_harfbuzz.cc.o')
    for obj in objects:
        record = json.loads(obj.with_suffix('.json').read_text(encoding='utf-8'))
        for path, stamp in record['deps'].items():
            dependency = Path(path) if Path(path).is_absolute() else root/path
            st = dependency.stat()
            if [st.st_mtime_ns, st.st_size] != stamp:
                parser.error('Stale production object; rebuild normally before testing: '+str(obj))
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%SZ')
    out = (args.out_dir or root/'build'/('document-ime-bridge-'+stamp+'-'+str(os.getpid()))).resolve()
    if out.exists():
        parser.error('--out-dir must be new')
    out.mkdir(parents=True)
    files = [Path(__file__).resolve(), root/'tests/xui/xui_document_ime_bridge_test.c',
             root/'tests/xui/xui_test_proxy.c', root/'tests/xui/xui_test_proxy.h', runtime/'build-report.json']+objects
    # The engine/backend is compiled into the test so its private text store
    # can be driven through the real COM vtables. All XUI code uses the normal
    # production objects; there are no test-only exports or copied adapters.
    report = dict(status='running', input_sha256={str(p): sha(p) for p in files}, checks=[],
                  production_objects_dependency_stamps_verified=True, physical_ime_verified=False,
                  rendering_uses_deterministic_proxy=True, user_clipboard_accessed=False,
                  physical_gpu_verified=False, overall_goal_complete=False)

    def save():
        (out/'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')

    save()
    try:
        exe = out/'document-ime-bridge.exe'
        command = [os.environ.get('CC', 'gcc'), *build['flags'], '-O0',
                   'tests/xui/xui_document_ime_bridge_test.c', 'tests/xui/xui_test_proxy.c',
                   *map(str, objects), '-lm', '-lws2_32', '-liphlpapi', '-lgdi32', '-luser32',
                   '-lshell32', '-lole32', '-loleaut32', '-luuid', '-limm32', '-lwinmm', '-lavrt', '-o', str(exe)]
        for name, args in [('build', command), ('run', [str(exe)])]:
            row = dict(name=name, command=args, log=str(out/(name+'.log')))
            report['checks'].append(row)
            with Path(row['log']).open('xb') as stream:
                result = subprocess.run(args, cwd=root, stdout=stream, stderr=subprocess.STDOUT, timeout=180)
            row.update(actual_exit=result.returncode, log_sha256=sha(row['log']))
            save()
            print(name+' actual EXIT '+str(result.returncode), flush=True)
            if result.returncode:
                print(Path(row['log']).read_text(encoding='utf-8', errors='replace')[-4000:])
                raise RuntimeError(name+' failed')
        report['status'] = 'passed'
    except BaseException as error:
        report.update(status='failed', error=str(error))
        raise
    finally:
        report['input_changes'] = [p for p, h in report['input_sha256'].items() if sha(p) != h]
        save()
    if report['input_changes']:
        raise RuntimeError('Test inputs changed during execution')


if __name__ == '__main__':
    main()
