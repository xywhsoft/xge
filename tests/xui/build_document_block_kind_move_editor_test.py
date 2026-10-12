"""Test lossless block moves in an existing shared-library Editor (proxy backend)."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import subprocess
import sys
sys.stdout.reconfigure(encoding="utf-8")

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source-root', type=Path, default=Path(__file__).resolve().parents[2])
parser.add_argument('--build-dir', type=Path, required=True, help='Matching shared library and xge_build_config.h directory.')
parser.add_argument('--out-dir', type=Path, required=True, help='New output directory; previous results remain intact.')
args = parser.parse_args()
if sys.platform not in ('win32', 'linux'): parser.error('This entry is currently verified on Windows and Linux.')
root, build, folder = args.source_root.resolve(), args.build_dir.resolve(), args.out_dir.resolve()
test_dir = Path(__file__).resolve().parent
library = build / ('xge.dll' if os.name == 'nt' else 'libxge.so')
config = build / 'xge_build_config.h'
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
paths = [Path(__file__), library, config, test_dir / 'xui_document_block_kind_move_test.c',
    test_dir / 'xui_document_block_kind_move_editor_test.c', root / 'tests/xui/xui_test_proxy.c',
    root / 'tests/xui/xui_test_proxy.h', root / 'src/xui/core/xui_unicode.c', root / 'include/xui_document.h',
    root / 'include/xui_document_ui.h', root / 'include/xui.h', root / 'include/xui_config.h', root / 'include/xge_config.h']
if os.name == 'nt': paths.append(build / 'xge.lib')
inputs = {str(p): sha(p) for p in paths}
folder.mkdir(parents=True)
with (folder / 'inputs.json').open('x', encoding='utf-8') as stream: json.dump(inputs, stream, indent=2)
flags = ['-std=c11', '-O2', '-g', '-Wall', '-Wextra', '-Werror', '-Wno-unused-function',
    '-DXGE_DLL', '-DXUI_DLL', '-I' + str(root / 'include'), '-I' + str(root), '-I' + str(test_dir), '-include', str(config)]
links = ([build / 'xge.lib', '-lm', '-lws2_32', '-liphlpapi', '-lgdi32', '-luser32', '-lshell32',
    '-lole32', '-lwinmm', '-lavrt'] if os.name == 'nt' else [library, '-lm', '-ldl', '-lpthread'])
env = os.environ.copy()
key = 'PATH' if os.name == 'nt' else 'LD_LIBRARY_PATH'
env[key] = str(build) + os.pathsep + env.get(key, '')
checks = []
def run(name, command):
    command = [str(p) for p in command]
    log = folder / (name + '.log')
    with log.open('x', encoding='utf-8') as stream:
        result = subprocess.run(command, cwd=folder, env=env, stdout=stream, stderr=subprocess.STDOUT)
    checks.append({'name': name, 'command': command, 'actual_exit': result.returncode,
        'log': str(log), 'log_sha256': sha(log)})
    (folder / 'checks.json').write_text(json.dumps(checks, indent=2), encoding='utf-8')
    if result.returncode:
        print(log.read_text(encoding='utf-8', errors='replace')[-7000:], flush=True)
        raise SystemExit(result.returncode)
    return log
exe = folder / ('document-block-kind-editor.exe' if os.name == 'nt' else 'document-block-kind-editor')
run('compile', ['gcc', *flags, test_dir / 'xui_document_block_kind_move_editor_test.c',
    root / 'tests/xui/xui_test_proxy.c', root / 'src/xui/core/xui_unicode.c', *links, '-o', exe])
log = run('run', [exe])
text = log.read_text(encoding='utf-8', errors='replace')
assert 'Block Editor: 243 actual DLL external-Document-move/selection/source/HTML/all-NodeId/shared-history/three-mode cases passed; proxy backend.' in text
assert all(sha(Path(p)) == h for p, h in inputs.items())
with (folder / 'report.json').open('x', encoding='utf-8') as stream:
    json.dump({'platform': sys.platform, 'checks': checks, 'inputs_sha256': sha(folder / 'inputs.json'),
        'library_sha256': sha(library), 'executable_sha256': sha(exe), 'editor_cases': 243, 'external_document_move_observer': True,
        'inputs_unchanged': True, 'backend': 'proxy', 'physical_platform_acceptance': False,
        'full_document_acceptance': False}, stream, indent=2)
print(text.rstrip(), flush=True)
