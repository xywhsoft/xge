"""Build a fresh Document kernel and verify lossless deletion across all seven nonparagraph list-header kinds."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import re
import subprocess
import sys

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source-root', type=Path, default=Path(__file__).resolve().parents[2])
parser.add_argument('--out-dir', type=Path, required=True)
parser.add_argument('--variant', choices=['normal', 'collision'], default='normal')
parser.add_argument('--sanitize', action=argparse.BooleanOptionalAction, default=sys.platform.startswith('linux'))
args = parser.parse_args()
if sys.platform not in ('win32', 'linux'):
    parser.error('This entry currently supports Windows and Linux GCC.')
if os.name == 'nt' and args.sanitize:
    parser.error('This Windows GCC configuration does not support sanitizers.')
root, folder = args.source_root.resolve(), args.out_dir.resolve()
folder.mkdir(parents=True, exist_ok=False)
temporary = folder / 'compiler-tmp'
temporary.mkdir()
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
sources = json.loads((root / 'config/modules.json').read_text(encoding='utf-8'))['source_groups']['document-core'].copy()
sources.append('tests/xui/xui_test_xrt_impl.c')
assert len(sources) == 29 and len(sources) == len(set(sources))
tests = ['xui_document_nonparagraph_prefix_cst_test', 'xui_document_nonparagraph_long_header_test', 'xui_document_nonparagraph_prefix_fault_test',
    'xui_document_inline_range_cst_test', 'xui_document_delete_source_cst_test', 'xui_document_container_prefix_cst_test']
paths = {root / p for p in sources}
paths.update(root.glob('include/*.h'))
paths.update(root.glob('src/**/*.h'))
paths.update(root.glob('extlibs/xrt/*.h'))
paths.update(root.glob('extlibs/md4c/*.h'))
paths.update(root.glob('tests/xui/*.c'))
paths.update(root.glob('tests/xui/*.h'))
paths.update([root / 'extlibs/md4c/md4c.c', root / 'tools/build/xui_document_sources.bat', Path(__file__).resolve()])
inputs = {str(p): sha(p) for p in sorted(paths)}
with (folder / 'inputs.json').open('x', encoding='utf-8') as stream:
    json.dump(inputs, stream, indent=2)
flags = ['-std=c11', '-O1' if args.sanitize else '-O2', '-g', '-Wall', '-Wextra', '-Werror',
    '-Wno-unused-function', '-I' + str(root / 'include'), '-I' + str(root)]
if args.variant == 'collision':
    flags.append('-DXUI_DOC_REFERENCE_HASH_COLLISION_TEST')
if args.sanitize:
    flags.extend(['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-no-pie'])
links = ['-lm', '-lws2_32', '-liphlpapi'] if os.name == 'nt' else ['-lm', '-ldl', '-lpthread']
env = os.environ.copy()
env['TMPDIR'] = str(temporary)
if os.name == 'nt':
    env['TMP'] = env['TEMP'] = str(temporary)
if args.sanitize:
    env['ASAN_OPTIONS'] = 'detect_leaks=1:halt_on_error=1'
    env['UBSAN_OPTIONS'] = 'halt_on_error=1'
checks = []

def unchanged():
    assert all(sha(Path(p)) == h for p, h in inputs.items()), 'A protected input changed.'

def run(name, command):
    unchanged()
    command = [str(p) for p in command]
    log = folder / (name + '.log')
    with log.open('x', encoding='utf-8') as stream:
        result = subprocess.run(command, cwd=root, env=env, stdout=stream, stderr=subprocess.STDOUT)
    checks.append({'name': name, 'command': command, 'actual_exit': result.returncode,
        'log': str(log), 'log_sha256': sha(log)})
    (folder / 'checks.json').write_text(json.dumps(checks, indent=2), encoding='utf-8')
    unchanged()
    text = log.read_text(encoding='utf-8', errors='replace')
    if result.returncode:
        print(text[-7000:], flush=True)
        raise SystemExit(result.returncode)
    return text

objects = []
for relative in sources:
    obj = folder / (Path(relative).stem + '.o')
    assert not obj.exists()
    objects.append(obj)
    run('compile-' + Path(relative).stem, ['gcc', *flags, '-c', root / relative, '-o', obj])
assert len({p.name for p in objects}) == len(objects)
archive = folder / 'document.a'
run('archive', ['ar', 'rcs', archive, *objects])
members = run('archive-members', ['ar', 't', archive]).splitlines()
assert members == [p.name for p in objects]
for name in tests:
    executable = folder / (name + ('.exe' if os.name == 'nt' else ''))
    run(name + '-compile', ['gcc', *flags, root / ('tests/xui/' + name + '.c'), archive, *links, '-o', executable])
    output = run(name + '-run', [executable])
    if name == 'xui_document_nonparagraph_prefix_fault_test':
        rows = re.findall(r'Nonparagraph prefix fault kind=(\d+) variant=(\d+) fixture=(\d+) command=(\d+) compound=(\d+) cancel=(\d+): (\d+) points;', output)
        assert len(rows) == 112 and all(int(row[-1]) > 0 for row in rows)
        assert {tuple(int(v) for v in row[1:-1]) for row in rows} == {
            (v, f, 0, t, x) for v in range(7) for f in range(4) for t in range(2) for x in range(2)}
        assert 'Nonparagraph prefix faults: 112 exhaustive OOM/cancel scans passed' in output
    elif name == 'xui_document_nonparagraph_prefix_cst_test':
        assert '5376 valid independent source/scope goldens, 0 rejected, 0 raw differences' in output
    elif name == 'xui_document_nonparagraph_long_header_test':
        assert '1008 valid independent source/scope goldens, 0 rejected, 0 raw differences' in output
    print(output.splitlines()[-1], flush=True)
unchanged()
with (folder / 'report.json').open('x', encoding='utf-8') as stream:
    json.dump({'platform': sys.platform, 'variant': args.variant, 'sanitizers': args.sanitize,
        'inputs_sha256': sha(folder / 'inputs.json'), 'checks': checks,
        'fresh_kernel_units': len(objects), 'reused_kernel_units': 0,
        'source_cases': 5376, 'long_header_cases': 1008, 'fault_scan_count': len(rows),
        'fault_points': sum(int(row[-1]) for row in rows),
        'library_sha256': sha(archive), 'object_sha256': {str(p): sha(p) for p in objects},
        'inputs_unchanged': True, 'status': 'passed', 'full_document_acceptance': False}, stream, indent=2)
print('Fresh nonparagraph prefix entry passed:', folder / 'report.json', flush=True)
