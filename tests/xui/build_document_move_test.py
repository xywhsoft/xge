"""Fresh standalone Document build and lossless block-move regression tests."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import subprocess
import sys

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source-root', type=Path, default=Path(__file__).resolve().parents[2])
parser.add_argument('--out-dir', type=Path, required=True, help='New directory; previous results are never overwritten.')
parser.add_argument('--variant', choices=['normal', 'collision'], default='normal')
parser.add_argument('--sanitize', action=argparse.BooleanOptionalAction, default=sys.platform.startswith('linux'))
args = parser.parse_args()
root = args.source_root.resolve()
folder = args.out_dir.resolve()
test_dir = Path(__file__).resolve().parent
if os.name == 'nt' and args.sanitize: parser.error('The Windows GCC entry does not support this sanitizer configuration.')
if sys.platform not in ('win32', 'linux'): parser.error('This build entry is currently verified on Windows and Linux.')
folder.mkdir(parents=True)
tmp = folder / 'compiler-tmp'
tmp.mkdir()
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
sources = json.loads((root / 'config/modules.json').read_text(encoding='utf-8'))['source_groups']['document-core'].copy()
assert sources and len(sources) == len(set(sources))
sources.append('tests/xui/xui_test_xrt_impl.c')
input_paths = {root / p for p in sources}
input_paths.update(root.glob('src/xui/document/xui_document*.h'))
input_paths.update(root.glob('src/xui/core/xui_unicode*.h'))
input_paths.update(root.glob('extlibs/xrt/*.h'))
input_paths.update(root.glob('extlibs/md4c/*.h'))
input_paths.update(root / p for p in ['include/xui_document.h', 'tools/build/xui_document_sources.bat', 'include/xui.h', 'include/xui_config.h', 'include/xge_config.h'])
input_paths.update(test_dir / p for p in ['build_document_move_test.py', 'xui_document_move_cases.h',
    'xui_document_move_test.c', 'xui_document_move_fault_test.c'])
inputs = {str(p): sha(p) for p in sorted(input_paths)}
with (folder / 'inputs.json').open('x', encoding='utf-8') as stream: json.dump(inputs, stream, indent=2)
flags = ['-std=c11', '-O1' if args.sanitize else '-O2', '-g', '-Wall', '-Wextra', '-Werror',
    '-Wno-unused-function', '-I' + str(root / 'include'), '-I' + str(root)]
if args.variant == 'collision': flags.append('-DXUI_DOC_REFERENCE_HASH_COLLISION_TEST')
if args.sanitize: flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-no-pie']
links = ['-lm', '-lws2_32', '-liphlpapi'] if os.name == 'nt' else ['-lm', '-ldl', '-lpthread']
env = os.environ.copy()
env['TMPDIR'] = str(tmp)
if os.name == 'nt': env['TMP'] = env['TEMP'] = str(tmp)
if args.sanitize:
    env['ASAN_OPTIONS'] = 'detect_leaks=1:halt_on_error=1'
    env['UBSAN_OPTIONS'] = 'halt_on_error=1'
checks = []
def run(name, command):
    command = [str(p) for p in command]
    log = folder / (name + '.log')
    with log.open('x', encoding='utf-8') as stream:
        result = subprocess.run(command, cwd=root, env=env, stdout=stream, stderr=subprocess.STDOUT)
    checks.append({'name': name, 'command': command, 'actual_exit': result.returncode,
        'log': str(log), 'log_sha256': sha(log)})
    (folder / 'checks.json').write_text(json.dumps(checks, indent=2), encoding='utf-8')
    if result.returncode:
        print(log.read_text(encoding='utf-8', errors='replace')[-7000:], flush=True)
        raise SystemExit(result.returncode)
    return log
objects = []
for relative in sources:
    obj = folder / (Path(relative).stem + '.o')
    objects.append(obj)
    run('compile-' + Path(relative).stem, ['gcc', *flags, '-c', root / relative, '-o', obj])
library = folder / 'document.a'
run('archive', ['ar', 'rcs', library, *objects])
for name in ['xui_document_move_test', 'xui_document_move_fault_test']:
    exe = folder / (name + ('.exe' if os.name == 'nt' else ''))
    run(name + '-compile', ['gcc', *flags, test_dir / (name + '.c'), library, *links, '-o', exe])
    log = run(name + '-run', [exe])
    output = log.read_text(encoding='utf-8', errors='replace')
    expected = ('Move matrix: 336 independent source/HTML/identity/snapshot/Undo/Redo cases; 0 source differences, 0 rejections, 0 semantic failures, 0 invariant failures'
        if name == 'xui_document_move_test' else 'Move faults: 56 exhaustive OOM/cancel scans;')
    assert expected in output
    print(output.splitlines()[-1], flush=True)
assert all(sha(Path(p)) == h for p, h in inputs.items()), 'A protected input changed during the test.'
with (folder / 'report.json').open('x', encoding='utf-8') as stream:
    json.dump({'platform': sys.platform, 'variant': args.variant, 'sanitizers': args.sanitize,
        'inputs_sha256': sha(folder / 'inputs.json'), 'checks': checks,
        'fresh_kernel_units': len(objects), 'reused_kernel_units': 0,
        'source_cases': 336, 'fault_scan_count': 56, 'library_sha256': sha(library),
        'inputs_unchanged': True, 'full_document_acceptance': False}, stream, indent=2)
print('Document move entry passed:', folder / 'report.json', flush=True)
