"""Fresh standalone Document build and container-prefix/move/large-payload regression tests."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import subprocess
import sys
sys.stdout.reconfigure(encoding="utf-8")

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source-root', type=Path, default=Path(__file__).resolve().parents[1])
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
sources = []
for line in (root / 'xui_document_sources.bat').read_text(encoding='utf-8').splitlines():
    if line.startswith('set XUI_DOCUMENT_SRC='):
        sources.extend(p.replace('\\', '/') for p in line.split('=', 1)[1].replace('%XUI_DOCUMENT_SRC%', '').split())
assert sources and len(sources) == len(set(sources))
sources.append('test_xui/xui_test_xrt_impl.c')
input_paths = {root / p for p in sources}
input_paths.update(root.glob('src/xui_document*.h'))
input_paths.update(root.glob('src/xui_unicode*.h'))
input_paths.update(root.glob('lib/xrt/*.h'))
input_paths.update(root.glob('lib/md4c/*.h'))
input_paths.add(root / 'lib/md4c/md4c.c')
input_paths.update(root / p for p in ['xui_document.h', 'xui_document_sources.bat', 'xui.h', 'xui_config.h', 'xge_config.h'])
input_paths.update(test_dir / p for p in ['build_document_container_move_test.py', 'DOCUMENT_CONTAINER_MOVE_TESTS.md', 'xui_document_container_move_cases.h',
    'xui_document_container_move_test.c', 'xui_document_container_prefix_test.c', 'xui_document_container_move_fault_test.c',
    'xui_document_container_prefix_fault_test.c', 'xui_document_container_large_test.c', 'xui_document_slot_move_cases.h', 'xui_document_slot_nested_cases.h',
    'xui_document_slot_move_test.c', 'xui_document_slot_nested_test.c', 'xui_document_slot_move_fault_test.c', 'xui_document_slot_nested_fault_test.c',
    'xui_document_adjacent_move_cases.h', 'xui_document_adjacent_move_test.c', 'xui_document_adjacent_move_fault_test.c',
    'xui_document_block_kind_move_test.c', 'xui_document_block_kind_move_fault_test.c',
    'xui_document_sole_child_move_test.c', 'xui_document_sole_child_move_fault_test.c',
    'xui_document_complex_move_test.c', 'xui_document_complex_move_fault_test.c',
    'xui_document_code_tab_move_test.c', 'xui_document_code_tab_move_fault_test.c'])
inputs = {str(p): sha(p) for p in sorted(input_paths)}
with (folder / 'inputs.json').open('x', encoding='utf-8') as stream: json.dump(inputs, stream, indent=2)
flags = ['-std=c11', '-O1' if args.sanitize else '-O2', '-g', '-Wall', '-Wextra', '-Werror',
    '-Wno-unused-function', '-I' + str(root)]
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
entries = [
    ('xui_document_complex_move_test', 'Block kinds: 108 independent complex-container oracle variants; 0 fixture failures, 0 rejections, 0 raw differences, 0 semantic differences, 0 history differences'),
    ('xui_document_complex_move_fault_test', 'Block kind faults: 18 exhaustive OOM/cancel scans;'),
    ('xui_document_code_tab_move_test', 'Block kinds: 108 independent code-Tab-phase oracle variants; 0 fixture failures, 0 rejections, 0 raw differences, 0 semantic differences, 0 history differences'),
    ('xui_document_code_tab_move_fault_test', 'Block kind faults: 18 exhaustive OOM/cancel scans;'),
    ('xui_document_sole_child_move_test', 'Block kinds: 1260 independent sole-child oracle variants; 0 fixture failures, 0 rejections, 0 raw differences, 0 semantic differences, 0 history differences'),
    ('xui_document_sole_child_move_fault_test', 'Block kind faults: 66 exhaustive OOM/cancel scans;'),
    ('xui_document_block_kind_move_test', 'Block kinds: 324 independent oracle variants; 0 fixture failures, 0 rejections, 0 raw differences, 0 semantic differences, 0 history differences'),
    ('xui_document_block_kind_move_fault_test', 'Block kind faults: 54 exhaustive OOM/cancel scans;'),
    ('xui_document_adjacent_move_test', 'Container matrix: 72 independent source/HTML/all-NodeId/snapshot/Undo/Redo cases; 0 source differences, 0 semantic differences, 0 rejections, 0 history differences'),
    ('xui_document_adjacent_move_fault_test', 'Container faults: 12 exhaustive OOM/cancel scans;'),
    ('xui_document_slot_move_test', 'Container matrix: 72 independent source/HTML/all-NodeId/snapshot/Undo/Redo cases; 0 source differences, 0 semantic differences, 0 rejections, 0 history differences'),
    ('xui_document_slot_nested_test', 'Container matrix: 72 independent source/HTML/all-NodeId/snapshot/Undo/Redo cases; 0 source differences, 0 semantic differences, 0 rejections, 0 history differences'),
    ('xui_document_slot_move_fault_test', 'Container faults: 12 exhaustive OOM/cancel scans;'),
    ('xui_document_slot_nested_fault_test', 'Container faults: 12 exhaustive OOM/cancel scans;'),
    ('xui_document_container_move_test', 'Container matrix: 108 independent source/HTML/all-NodeId/snapshot/Undo/Redo cases; 0 source differences, 0 semantic differences, 0 rejections, 0 history differences'),
    ('xui_document_container_prefix_test', 'Container matrix: 72 independent source/HTML/all-NodeId/snapshot/Undo/Redo cases; 0 source differences, 0 semantic differences, 0 rejections, 0 history differences'),
    ('xui_document_container_move_fault_test', 'Container faults: 12 exhaustive OOM/cancel scans;'),
    ('xui_document_container_prefix_fault_test', 'Container faults: 12 exhaustive OOM/cancel scans;'),
    ('xui_document_container_large_test', 'Container large: six 1/10MiB source payload cases; all invariants passed'),
]
for name, expected in entries:
    exe = folder / (name + ('.exe' if os.name == 'nt' else ''))
    run(name + '-compile', ['gcc', *flags, test_dir / (name + '.c'), library, *links, '-o', exe])
    log = run(name + '-run', [exe])
    output = log.read_text(encoding='utf-8', errors='replace')
    assert expected in output
    print(output.splitlines()[-1], flush=True)

assert all(sha(Path(p)) == h for p, h in inputs.items()), 'A protected input changed during the test.'
with (folder / 'report.json').open('x', encoding='utf-8') as stream:
    json.dump({'platform': sys.platform, 'variant': args.variant, 'sanitizers': args.sanitize,
        'inputs_sha256': sha(folder / 'inputs.json'), 'checks': checks,
        'fresh_kernel_units': len(objects), 'reused_kernel_units': 0,
        'source_cases': 2196, 'complex_source_cases': 108, 'code_tab_source_cases': 108, 'sole_child_source_cases': 1260, 'container_source_cases': 396, 'block_kind_source_cases': 324, 'fault_scan_count': 216, 'complex_fault_scan_count': 18, 'code_tab_fault_scan_count': 18, 'sole_child_fault_scan_count': 66, 'large_payload_cases': 6, 'library_sha256': sha(library),
        'inputs_unchanged': True, 'full_document_acceptance': False}, stream, indent=2)
print('Document container entry passed:', folder / 'report.json', flush=True)
