"""Verify deep quote deletion and continued input in a matching DocumentEditor."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import subprocess
import sys

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source-root', type=Path, default=Path(__file__).resolve().parents[2])
parser.add_argument('--build-dir', type=Path, required=True)
parser.add_argument('--out-dir', type=Path, required=True)
args = parser.parse_args()
if sys.platform not in ('win32', 'linux'):
    parser.error('This entry currently supports Windows and Linux GCC.')
root, build, folder = args.source_root.resolve(), args.build_dir.resolve(), args.out_dir.resolve()
test_dir = Path(__file__).resolve().parent
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
library = build / ('xge.dll' if os.name == 'nt' else 'libxge.so')
config = build / 'xge_build_config.h'
test = test_dir / 'xui_document_deep_quote_editor_test.c'
paths = {Path(__file__).resolve(), test, root / 'tests/xui/xui_document_deep_quote_samples.h', library, config, root / 'src/xui/core/xui_unicode.c', root / 'tests/xui/xui_test_proxy.c'}
paths.update(root.glob('include/*.h'))
paths.update(root.glob('src/**/*.h'))
paths.update(root.glob('extlibs/xrt/*.h'))
paths.update(root.glob('tests/xui/*.c'))
paths.update(root.glob('tests/xui/*.h'))
if os.name == 'nt': paths.add(build / 'xge.lib')
inputs = {str(p): sha(p) for p in sorted(paths)}
folder.mkdir(parents=True, exist_ok=False)
with (folder / 'inputs.json').open('x', encoding='utf-8') as stream:
    json.dump(inputs, stream, indent=2)
flags = ['-std=c11', '-O2', '-g', '-Wall', '-Wextra', '-Werror', '-Wno-unused-function',
    '-DXGE_DLL', '-DXUI_DLL', '-I' + str(root / 'include'), '-I' + str(root), '-include', str(config)]
links = ([build / 'xge.lib', '-lm', '-lws2_32', '-liphlpapi', '-lgdi32', '-luser32', '-lshell32',
    '-lole32', '-lwinmm', '-lavrt'] if os.name == 'nt' else [library, '-lm', '-ldl', '-lpthread'])
env = os.environ.copy()
key = 'PATH' if os.name == 'nt' else 'LD_LIBRARY_PATH'
env[key] = str(build) + os.pathsep + env.get(key, '')
env['TMPDIR'] = str(folder)
if os.name == 'nt': env['TMP'] = env['TEMP'] = str(folder)
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
    output = log.read_text(encoding='utf-8', errors='replace')
    if result.returncode:
        print(output[-7000:], flush=True)
        raise SystemExit(result.returncode)
    return output

executable = folder / ('deep-quote-editor.exe' if os.name == 'nt' else 'deep-quote-editor')
run('compile', ['gcc', *flags, test, root / 'tests/xui/xui_test_proxy.c', root / 'src/xui/core/xui_unicode.c', *links, '-o', executable])
output = run('editor432', [executable])
assert 'Deep quote Editor: 432 external-single/compound three-mode and visual Delete/Backspace/empty-replacement/continued-input shared-history cases passed; proxy backend.' in output
with (folder / 'report.json').open('x', encoding='utf-8') as stream:
    json.dump({'platform': sys.platform, 'checks': checks, 'inputs_sha256': sha(folder / 'inputs.json'),
        'library_sha256': sha(library), 'executable_sha256': sha(executable), 'editor_cases': 432,
        'external_single_and_compound_range_delete_observer': True, 'visual_delete_backspace_empty_replacement_and_continued_input': True,
        'shared_history_and_three_modes': True, 'inputs_unchanged': True, 'backend': 'proxy',
        'physical_platform_acceptance': False, 'full_document_acceptance': False, 'status': 'passed'}, stream, indent=2)
print(output.splitlines()[-1], flush=True)
