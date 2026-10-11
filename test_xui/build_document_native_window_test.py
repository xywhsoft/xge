#!/usr/bin/env python3
"""Compile GL header coexistence and run the public Document API on actual XGE."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    tests = Path(__file__).resolve().parent
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-root', type=Path, default=tests.parent)
    parser.add_argument('--build-dir', type=Path, required=True,
                        help='Existing profile output containing library and xge_build_config.h')
    parser.add_argument('--out-dir', type=Path, required=True, help='New directory for logs and images')
    parser.add_argument('--font-dir', type=Path, help='Directory containing the four DejaVu fonts')
    parser.add_argument('--headless', action='store_true', help='Linux: isolated Xvfb and software GL')
    args = parser.parse_args()
    windows = sys.platform == 'win32'
    if not windows and sys.platform != 'linux':
        parser.error('This entry currently verifies Windows and Linux only')
    if args.headless and windows:
        parser.error('--headless is Linux-only')
    root, build, out = (p.resolve() for p in (args.source_root, args.build_dir, args.out_dir))
    compiler = shutil.which(os.environ.get('CC', 'gcc'))
    if not compiler:
        parser.error('C compiler not found; set CC')
    if args.headless and not shutil.which('xvfb-run'):
        parser.error('xvfb-run is required for --headless')
    if args.font_dir:
        font_root = args.font_dir.resolve()
        names = ['DejaVuSans.ttf', 'DejaVuSans-Bold.ttf', 'DejaVuSans-Oblique.ttf', 'DejaVuSansMono.ttf']
    elif windows:
        font_root = Path(os.environ.get('WINDIR', r'C:\Windows')) / 'Fonts'
        names = ['arial.ttf', 'arialbd.ttf', 'ariali.ttf', 'consola.ttf']
    else:
        font_root = Path('/usr/share/fonts/truetype/dejavu')
        names = ['DejaVuSans.ttf', 'DejaVuSans-Bold.ttf', 'DejaVuSans-Oblique.ttf', 'DejaVuSansMono.ttf']
    fonts = [font_root / name for name in names]
    library = build / ('xge.lib' if windows else 'libxge.so')
    runtime = build / ('xge.dll' if windows else 'libxge.so')
    config = build / 'xge_build_config.h'
    source = tests / 'xui_document_native_window_test.c'
    header_source = tests / 'xge_gl_system_header_test.c'
    input_paths = [Path(__file__).resolve(), source, header_source, config, library, runtime, *fonts]
    missing = [str(p) for p in input_paths if not p.is_file()]
    if missing:
        parser.error('Missing inputs: ' + ', '.join(missing))
    if out.exists():
        parser.error('--out-dir must be new; preserve previous failures and artifacts')
    inputs = {str(p): sha(p) for p in input_paths}
    out.mkdir(parents=True)
    env = os.environ.copy()
    if windows:
        env['PATH'] = str(build) + os.pathsep + env.get('PATH', '')
    else:
        env['LD_LIBRARY_PATH'] = str(build) + os.pathsep + env.get('LD_LIBRARY_PATH', '')
    rows = []
    report = {'platform': sys.platform, 'source_root': str(root), 'build_dir': str(build),
              'input_sha256': inputs, 'headless_software_gl': args.headless,
              'physical_ime_dpi_reader_verification': False, 'checks': rows, 'status': 'running'}

    def save():
        (out / 'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')

    def run(name, command):
        log = out / (name + '.log')
        with log.open('x', encoding='utf-8') as stream:
            try:
                result = subprocess.run([str(x) for x in command], cwd=root, env=env,
                                        stdout=stream, stderr=subprocess.STDOUT, timeout=180)
                code = result.returncode
            except subprocess.TimeoutExpired:
                code = 124
        rows.append({'name': name, 'command': [str(x) for x in command], 'actual_exit': code,
                     'log_sha256': sha(log)})
        save()
        print(name, 'actual EXIT', code, flush=True)
        text = log.read_text(encoding='utf-8', errors='replace')
        if code:
            print(text[-6000:], flush=True)
            report['status'] = 'failed'; save()
            raise SystemExit(code)
        return text

    flags = [compiler, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
             '-DXGE_DLL', '-DXUI_DLL', '-include', str(config), '-I' + str(root)]
    for system_first in [False, True]:
        name = 'system-gl-first' if system_first else 'private-gl-first'
        extra = ['-DTEST_SYSTEM_GL_FIRST'] if system_first else []
        run(name, [*flags, *extra, '-c', header_source, '-o', out / (name + '.o')])
    executable = out / ('document-native-window.exe' if windows else 'document-native-window')
    links = [library, '-lm', '-lopengl32'] if windows else [library, '-lm', '-lGL', '-Wl,-rpath,' + str(build)]
    run('document-compile', [*flags, source, *links, '-MD', '-MF', out / 'document.d', '-o', executable])
    command = [executable, out, *fonts]
    if args.headless:
        command = ['xvfb-run', '-a', '-s', '-screen 0 1024x768x24',
                   'env', 'LIBGL_ALWAYS_SOFTWARE=1', *command]
    output = run('document-run', command)
    assert 'Actual XGE GL context:' in output
    assert 'Native Document: 12 real RGBA frames/12 geometry checks/4 PNGs;' in output
    assert 'Compiled/actual build features match:' in output
    assert all(sha(Path(p)) == h for p, h in inputs.items()), 'Inputs changed during verification'
    report['images'] = []
    for name in ['native-rich.png', 'native-markdown.png', 'native-source.png', 'native-live.png']:
        path = out / name
        assert path.read_bytes()[:8] == b'\x89PNG\r\n\x1a\n'
        report['images'].append({'name': name, 'sha256': sha(path)})
    report['executable_sha256'] = sha(executable)
    report['status'] = 'passed'; save()
    print(output.rstrip(), flush=True)
    print('Native window entry: four actual commands passed; physical IME/DPI/readers remain separate.', flush=True)


if __name__ == '__main__':
    main()
