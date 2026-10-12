#!/usr/bin/env python3
"""Run the five lossless move Editor entries against one matching shared library."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

tests = Path(__file__).resolve().parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source-root', type=Path, default=tests.parent)
parser.add_argument('--build-dir', type=Path, required=True)
parser.add_argument('--out-dir', type=Path, required=True, help='New output directory')
args = parser.parse_args()
if sys.platform not in ('win32', 'linux'):
    parser.error('The Editor regression entries currently support Windows and Linux')
root, build, out = (p.resolve() for p in (args.source_root, args.build_dir, args.out_dir))
if out.exists():
    parser.error('--out-dir must be new')
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
library = build / ('xge.dll' if os.name == 'nt' else 'libxge.so')
entries = [('block', 'move', 252), ('block-kind', 'block_kind_move', 243),
    ('sole-child', 'sole_child_move', 945), ('complex', 'complex_move', 81),
    ('code-tab', 'code_tab_move', 81)]
paths = [Path(__file__), library, build / 'xge_build_config.h']
paths.extend(tests / ('build_document_' + stem + '_editor_test.py') for _, stem, _ in entries)
if os.name == 'nt':
    paths.append(build / 'xge.lib')
missing = [str(p) for p in paths if not p.is_file()]
if missing:
    parser.error('Missing inputs: ' + ', '.join(missing))
inputs = {str(p): sha(p) for p in paths}
out.mkdir(parents=True)
report = {'status': 'running', 'source_root': str(root), 'build_dir': str(build),
    'input_sha256': inputs, 'library_sha256': sha(library), 'entries': [],
    'actual_product_checks': 0, 'editor_cases': 0, 'backend': 'proxy',
    'physical_platform_acceptance': False, 'full_document_acceptance': False}

def save():
    (out / 'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')

save()
try:
    for name, stem, cases in entries:
        command = [sys.executable, str(tests / ('build_document_' + stem + '_editor_test.py')),
            '--source-root', str(root), '--build-dir', str(build), '--out-dir', str(out / name)]
        log = out / (name + '.log')
        with log.open('x', encoding='utf-8') as stream:
            result = subprocess.run(command, cwd=root, stdout=stream, stderr=subprocess.STDOUT)
        entry = {'name': name, 'command': command, 'actual_exit': result.returncode,
            'log_sha256': sha(log)}
        report['entries'].append(entry)
        save()
        print(name, 'entry actual EXIT', result.returncode, flush=True)
        if result.returncode:
            print(log.read_text(encoding='utf-8', errors='replace')[-7000:], flush=True)
            raise SystemExit(result.returncode)
        child_path = out / name / 'report.json'
        child = json.loads(child_path.read_text(encoding='utf-8'))
        assert child['editor_cases'] == cases and child['backend'] == 'proxy'
        assert child['library_sha256'] == report['library_sha256'] and child['inputs_unchanged']
        assert len(child['checks']) == 2 and all(c['actual_exit'] == 0 for c in child['checks'])
        assert all(sha(Path(c['log'])) == c['log_sha256'] for c in child['checks'])
        child_inputs_path = out / name / 'inputs.json'
        assert sha(child_inputs_path) == child['inputs_sha256']
        child_inputs = json.loads(child_inputs_path.read_text(encoding='utf-8'))
        assert all(sha(Path(p)) == h for p, h in child_inputs.items())
        entry.update(report_sha256=sha(child_path), actual_product_checks=2, editor_cases=cases)
        report['actual_product_checks'] += 2
        report['editor_cases'] += cases
        save()
    assert report['actual_product_checks'] == 10 and report['editor_cases'] == 1602
    assert all(sha(Path(p)) == h for p, h in inputs.items())
    report['status'] = 'passed'
    save()
except BaseException as error:
    report['status'] = 'failed'
    report['error'] = str(error)
    save()
    raise
print('Move Editor suite: ten actual shared-library checks,1602 three-mode cases;proxy backend.', flush=True)
