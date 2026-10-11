#!/usr/bin/env python3
"""Verify the native demo Find/Replace window in Rich and all Markdown modes."""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import secrets
import selectors
import shutil
import subprocess
import sys
import time


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out-dir', type=Path, help='New directory for the executable, logs and four workflow image sets')
    parser.add_argument('--runtime-dir', type=Path, help='Existing normal XGE runtime; this entry never rebuilds it')
    parser.add_argument('--cjk-font', type=Path, help='Existing local Chinese fallback font')
    parser.add_argument('--with-demo-verification', action='store_true', help='Also rebuild the original native example and run its four smoke checks')
    parser.add_argument('--with-format', action='store_true', help='Also validate the native Format menu, command availability and history')
    args = parser.parse_args()
    windows = sys.platform == 'win32'
    if not windows and not sys.platform.startswith('linux'):
        parser.error('This native entry currently supports Windows and Linux.')
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%SZ')
    out = (args.out_dir or root / 'build' / ('document-find-' + stamp + '-' + str(os.getpid()))).resolve()
    if out.exists():
        parser.error('--out-dir must be new so failures and prior evidence are preserved')
    runtime = (args.runtime_dir or root / ('build' if windows else 'build/linux-native')).resolve()
    required = ['xge.dll', 'xge.lib', 'xge_build_config.h'] if windows else ['libxge.so', 'xge_build_config.h']
    if any(not (runtime / name).is_file() for name in required):
        parser.error('--runtime-dir must contain the normal runtime and its build configuration')
    if args.cjk_font and not args.cjk_font.is_file():
        parser.error('--cjk-font must name an existing local font')
    out.mkdir(parents=True)
    env = dict(os.environ)
    if args.cjk_font:
        env['XUI_DOCUMENT_CJK_FONT'] = str(args.cjk_font.resolve())
    files = [Path(__file__).resolve(), root/'examples/xui_document/main.c',
             root/'test_xui/xui_document_demo_find_test.c', root/'xui.h', root/'xui_document.h',
             root/'xui_document_ui.h', root/'xge.h']
    inputs = {str(p): sha(p) for p in files}
    products = {str(runtime/name): sha(runtime/name) for name in required}
    report = dict(status='running', checks=[], input_sha256=inputs, runtime_sha256=products,
                  source_root=str(root), started_at=time.time(), user_clipboard_accessed=False,
                  user_display_accessed=False, physical_ime_verified=False, physical_gpu_verified=False,
                  physical_keyboard_mouse_verified=False, advanced_matrix_requested=False,
                  native_clipboard_success_verified=False, overall_design_complete=False)
    report['format_menu_requested'] = args.with_format
    font = env.get('XUI_DOCUMENT_CJK_FONT')
    if font:
        report['configured_font'] = dict(path=font, sha256=sha(font))
    server = None
    auth = out / 'Xauthority'

    def save():
        (out / 'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')

    def check(name, command, cwd):
        row = dict(name=name, command=list(map(str, command)), cwd=str(cwd), log=str(out/(name+'.log')))
        report['checks'].append(row)
        with Path(row['log']).open('xb') as stream:
            try:
                result = subprocess.run(row['command'], cwd=cwd, env=env, stdout=stream,
                                        stderr=subprocess.STDOUT, timeout=90)
            except subprocess.TimeoutExpired:
                row.update(status='timed_out', log_sha256=sha(row['log']))
                save()
                raise
        row.update(actual_exit=result.returncode, log_sha256=sha(row['log']))
        save()
        print(name + ' actual EXIT ' + str(result.returncode), flush=True)
        if result.returncode:
            raise RuntimeError(name + ' failed; see ' + row['log'])
        return Path(row['log']).read_text(encoding='utf-8', errors='replace')

    save()
    try:
        executable = out / ('document-find.exe' if windows else 'document-find')
        command = [os.environ.get('CC', 'gcc'), '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                   '-I.', '-DXGE_DLL', '-DXUI_DLL', '-o', executable, 'test_xui/xui_document_demo_find_test.c']
        if windows:
            command += [runtime/'xge.lib', '-lm', '-lws2_32', '-liphlpapi', '-lgdi32', '-luser32',
                        '-lshell32', '-lole32', '-lwinmm', '-lavrt']
        else:
            command += ['-L' + str(runtime), '-Wl,-rpath,' + str(runtime), '-lxge', '-lm']
        check('build', command, root)
        if windows:
            shutil.copy2(runtime/'xge.dll', out/'xge.dll')
            assert sha(out/'xge.dll') == products[str(runtime/'xge.dll')]
            system = Path(env.get('SystemRoot', r'C:\Windows'))
            env['PATH'] = str(system/'System32') + os.pathsep + str(system)
            report['system_only_execution_path'] = True
        else:
            env.update(LC_ALL='C.UTF-8', LANG='C.UTF-8', LIBGL_ALWAYS_SOFTWARE='1',
                       GALLIUM_DRIVER='llvmpipe', LP_NUM_THREADS='1', XAUTHORITY=str(auth))
            dependency = check('dependencies', ['ldd', executable], root)
            if str(runtime/'libxge.so') not in dependency or 'not found' in dependency:
                raise RuntimeError('Executable did not resolve the requested normal runtime')
            cookie = secrets.token_hex(16)
            subprocess.run(['xauth', '-f', str(auth), 'add', ':0', '.', cookie], check=True,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            rd, wr = os.pipe()
            try:
                with (out/'Xvfb.log').open('xb') as stream:
                    server = subprocess.Popen(['Xvfb', '-displayfd', str(wr), '-screen', '0', '1280x780x24',
                                               '-nolisten', 'tcp', '-auth', str(auth)], pass_fds=(wr,),
                                              stdout=stream, stderr=subprocess.STDOUT)
                os.close(wr)
                wr = None
                with selectors.DefaultSelector() as selector:
                    selector.register(rd, selectors.EVENT_READ)
                    if not selector.select(15):
                        raise RuntimeError('Private Xvfb did not start')
                    number = os.read(rd, 128).decode().strip()
                if not number.isdecimal():
                    raise RuntimeError('Invalid private display number')
            finally:
                os.close(rd)
                if wr is not None:
                    os.close(wr)
            env['DISPLAY'] = ':' + number
            if env['DISPLAY'] == os.environ.get('DISPLAY'):
                raise RuntimeError('Refusing the invoking desktop display')
            subprocess.run(['xauth', '-f', str(auth), 'add', env['DISPLAY'], '.', cookie], check=True,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            report['private_display'] = env['DISPLAY']
        total = 0
        format_total = 0
        for mode in ('rich', 'source', 'visual', 'live'):
            directory = out/mode
            directory.mkdir()
            command = [executable, directory, mode]
            if args.with_format:
                command.append('format')
            log = check(mode, command, directory)
            match = re.search(r'Native Document find ' + mode + r': (\d+) checks passed\.', log)
            if not match:
                raise RuntimeError('Native find workflow did not reach its terminal phase: ' + mode)
            total += int(match[1])
            if args.with_format:
                formatted = re.search(r'Native Document format ' + mode + r': 13 commands, (\d+) checks passed\.', log)
                if not formatted:
                    raise RuntimeError('Native Format menu workflow did not reach its terminal phase: ' + mode)
                format_total += int(formatted[1])
        images = sorted(out.glob('*/*.png'))
        expected_images = 28 if args.with_format else 20
        if len(images) != expected_images:
            raise RuntimeError('Native workflow image count does not match the requested find/format scope')
        if args.with_demo_verification:
            directory = out/'native-demo'
            directory.mkdir()
            (directory/'artifacts/xui-document-rebuild').mkdir(parents=True)
            (directory/'res').mkdir()
            for resource in (root/'res').glob('msgbox_*.png'):
                shutil.copy2(resource, directory/'res'/resource.name)
            example = directory/('xui_document.exe' if windows else 'xui_document')
            if windows:
                compiler_env = dict(env)
                env['PATH'] = os.environ.get('PATH', '')
                check('example-build', ['cmd.exe', '/d', '/c', str(root/'examples/xui_document/build.bat')], root)
                shutil.copy2(root/'build/xui_document.exe', example)
                shutil.copy2(runtime/'xge.dll', directory/'xge.dll')
                env.clear()
                env.update(compiler_env)
            else:
                check('example-build', [sys.executable, root/'examples/xui_document/build_linux.py',
                                        '--runtime-dir', runtime, '--output', example], root)
            for flag in ('--verify', '--verify-async', '--verify-image-placeholder', '--verify-files'):
                check('example'+flag, [example, flag], directory)
            report.update(native_example_smoke_passed=True, native_example_sha256=sha(example),
                          native_demo_images=[dict(path=str(p), sha256=sha(p)) for p in
                                              sorted((directory/'artifacts/xui-document-rebuild').glob('*.png'))])
        if any(sha(p) != h for p, h in inputs.items()) or any(sha(p) != h for p, h in products.items()):
            raise RuntimeError('Sources or runtime changed during verification')
        report.update(status='passed', finished_at=time.time(), source_inputs_unchanged=True,
                      runtime_unchanged=True, completed_checks=total, completed_modes=4, native_renderer_verified=True,
                      synthetic_pointer_text_input=True, find_replace_history_focus_verified=True,
                      format_menu_verified=args.with_format,
                      completed_format_checks=format_total, completed_find_checks=total-format_total,
                      logical_format_popup_bounds_verified=args.with_format,
                      images=[dict(path=str(p), sha256=sha(p)) for p in images], executable_sha256=sha(executable))
    except BaseException as error:
        report.update(status='failed', finished_at=time.time(), error=str(error))
        raise
    finally:
        if server:
            server.terminate()
            try:
                server.wait(timeout=15)
            except subprocess.TimeoutExpired:
                server.kill()
                server.wait(timeout=15)
            report.update(private_server_terminal=server.poll() is not None, private_server_actual_exit=server.returncode)
        if auth.exists():
            auth.unlink()
        report['authentication_file_removed'] = not auth.exists()
        save()
    print('Native Document find verification passed: ' + str(out), flush=True)


if __name__ == '__main__':
    main()
