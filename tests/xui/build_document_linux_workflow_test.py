#!/usr/bin/env python3
"""Test native Document file/close dialogs on an owned, authenticated Linux display."""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import secrets
import selectors
import signal
import subprocess
import sys
import time


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out-dir', type=Path, help='New directory for executables, logs and captured dialogs')
    parser.add_argument('--runtime-dir', type=Path, help='Existing normal/default Linux runtime; otherwise build one')
    parser.add_argument('--cjk-font', type=Path, help='Local Chinese fallback font, overriding XUI_DOCUMENT_CJK_FONT')
    parser.add_argument('--with-native-input', action='store_true', help='Also test queued X11 Unicode, shortcuts, pointer and window-close events')
    args = parser.parse_args()
    if not sys.platform.startswith('linux'):
        parser.error('Run on Linux/X11 (including WSL); Windows uses build_document_demo_workflow_test.py.')
    out = (args.out_dir or root/'build'/('document-linux-workflow-' + datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%SZ') + '-' + str(os.getpid()))).resolve()
    if out.exists():
        parser.error('--out-dir must be new so earlier evidence remains intact')
    out.mkdir(parents=True)
    runtime = args.runtime_dir.resolve() if args.runtime_dir else out/'runtime'
    auth = out/'Xauthority'
    server = None
    env = dict(os.environ, LC_ALL='C.UTF-8', LANG='C.UTF-8', LIBGL_ALWAYS_SOFTWARE='1',
               GALLIUM_DRIVER='llvmpipe', LP_NUM_THREADS='1', XAUTHORITY=str(auth))
    if args.cjk_font:
        env['XUI_DOCUMENT_CJK_FONT'] = str(args.cjk_font.resolve())
    if args.with_native_input:
        env['XUI_TEST_PRIVATE_DISPLAY'] = '1'
    paths = [Path(__file__).resolve(), root/'tests/xui/xui_document_demo_workflow_test.c',
             root/'examples/xui/xui_document/main.c', root / 'include/xui.h', root / 'include/xui_document.h',
             root / 'include/xui_document_ui.h', root / 'include/xge.h']
    if args.with_native_input:
        paths.append(root/'tests/xui/xui_document_demo_x11_input_test.c')
    inputs = {str(p): sha(p) for p in paths}
    report = dict(status='running', source_root=str(root), started_at=time.time(), runtime=str(runtime),
                  checks=[], input_sha256=inputs, user_display_accessed=False, user_clipboard_accessed=False,
                  synthetic_pointer_key_input=True, native_physical_gpu_verified=False,
                  physical_ime_verified=False, native_os_event_queue_verified=False,
                  advanced_matrix_requested=False, overall_design_complete=False)
    report.update(native_x11_input_requested=args.with_native_input,
                  native_x11_document_input_verified=False, completed_native_input_modes=0)

    def save():
        (out/'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')

    def check(name, command, timeout=180):
        command = list(map(str, command))
        log = out/(name+'.log')
        row = dict(name=name, command=command, log=str(log))
        report['checks'].append(row)
        started = time.monotonic()
        with log.open('xb') as stream:
            child = subprocess.Popen(command, cwd=root, env=env, stdout=stream, stderr=subprocess.STDOUT,
                                     start_new_session=True)
            try:
                result = child.wait(timeout=timeout)
            except subprocess.TimeoutExpired:
                try:
                    os.killpg(child.pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass
                child.wait(timeout=15)
                row.update(status='timed_out', actual_exit=child.returncode, log_sha256=sha(log))
                save()
                raise
        row.update(actual_exit=result, seconds=time.monotonic()-started, log_sha256=sha(log))
        save()
        print(name+' actual EXIT '+str(result), flush=True)
        if result:
            print(log.read_text(encoding='utf-8', errors='replace')[-6000:], flush=True)
            raise RuntimeError(name+' failed; see '+str(log))
        return log

    save()
    try:
        if not args.runtime_dir:
            check('normal-build', [sys.executable, root/'tools/build/build.py', 'default', runtime], 900)
            build = json.loads((runtime/'build-report.json').read_text())
            report.update(compiled_units=len(build['compiled']), reused_units=len(build['reused']))
        library, config = runtime/'libxge.so', runtime/'xge_build_config.h'
        if not library.is_file() or not config.is_file():
            raise RuntimeError('Runtime must contain libxge.so and xge_build_config.h from the normal builder')
        library_hash, config_hash = sha(library), sha(config)
        report.update(runtime_sha256=library_hash, config_sha256=config_hash)
        fonts = {key: value for key, value in env.items() if key in (
            'XUI_DOCUMENT_FONT', 'XUI_DOCUMENT_FONT_BOLD', 'XUI_DOCUMENT_FONT_ITALIC',
            'XUI_DOCUMENT_FONT_MONO', 'XUI_DOCUMENT_CJK_FONT')}
        if any(not Path(p).is_file() for p in fonts.values()):
            raise RuntimeError('Configured document font does not exist')
        report['configured_font_sha256'] = {key: dict(path=p, sha256=sha(p)) for key, p in fonts.items()}
        check('runtime-dependencies', ['ldd', library])
        exe = out/'xui_document_workflow_test'
        check('compile', ['gcc', '-std=c11', '-O2', '-g', '-Wall', '-Wextra', '-Werror', '@config/includes.rsp',
                          '-DXGE_DLL', '-DXUI_DLL', '-o', exe, 'tests/xui/xui_document_demo_workflow_test.c',
                          '-L'+str(runtime), '-Wl,-rpath,'+str(runtime), '-lxge', '-lm'])
        dependencies = check('test-dependencies', ['ldd', exe]).read_text()
        if str(library) not in dependencies or 'not found' in dependencies:
            raise RuntimeError('Test did not resolve the requested normal runtime')
        cookie = secrets.token_hex(16)
        subprocess.run(['xauth', '-f', str(auth), 'add', ':0', '.', cookie], check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        rd, wr = os.pipe()
        try:
            with (out/'Xvfb.log').open('xb') as stream:
                server = subprocess.Popen(['Xvfb', '-displayfd', str(wr), '-screen', '0', '1280x780x24',
                                           '-nolisten', 'tcp', '-auth', str(auth)], pass_fds=(wr,),
                                          stdout=stream, stderr=subprocess.STDOUT, start_new_session=True)
            os.close(wr)
            wr = None
            with selectors.DefaultSelector() as selector:
                selector.register(rd, selectors.EVENT_READ)
                if not selector.select(15):
                    raise RuntimeError('Private Xvfb display did not start')
                number = os.read(rd, 128).decode().strip()
            if not number.isdecimal():
                raise RuntimeError('Xvfb returned an invalid display number')
        finally:
            os.close(rd)
            if wr is not None:
                os.close(wr)
        env['DISPLAY'] = ':'+number
        if env['DISPLAY'] == os.environ.get('DISPLAY'):
            raise RuntimeError('Refusing to use the invoking display')
        subprocess.run(['xauth', '-f', str(auth), 'add', env['DISPLAY'], '.', cookie], check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        report['private_display'] = env['DISPLAY']
        for mode in ('source', 'visual', 'live'):
            for action in ('cancel', 'save', 'discard', 'clean', 'pending'):
                name = mode+'-'+action
                directory = out/name
                directory.mkdir()
                check(name, [exe, directory, mode, action])
        if args.with_native_input:
            input_exe = out/'xui_document_x11_input_test'
            check('native-input-compile', ['gcc', '-std=c11', '-O2', '-g', '-Wall', '-Wextra', '-Werror', '@config/includes.rsp',
                                         '-DXGE_DLL', '-DXUI_DLL', '-o', input_exe,
                                         'tests/xui/xui_document_demo_x11_input_test.c', '-L'+str(runtime),
                                         '-Wl,-rpath,'+str(runtime), '-lxge', '-lm', '-lX11'])
            dependencies = check('native-input-dependencies', ['ldd', input_exe]).read_text()
            if str(library) not in dependencies or 'not found' in dependencies:
                raise RuntimeError('Native input test did not resolve the requested normal runtime')
            for mode in ('source', 'visual', 'live'):
                name = 'native-input-'+mode
                directory = out/name
                directory.mkdir()
                check(name, [input_exe, directory, mode, '--isolated-display'])
            report.update(native_x11_document_input_verified=True, native_os_event_queue_verified=True,
                          completed_native_input_modes=3, native_input_executable_sha256=sha(input_exe))
        if any(sha(p) != h for p, h in inputs.items()):
            raise RuntimeError('Workflow source inputs changed during validation')
        if sha(library) != library_hash or sha(config) != config_hash:
            raise RuntimeError('Runtime or configuration changed during validation')
        report['images'] = [dict(path=str(p), sha256=sha(p)) for p in sorted(out.glob('*/*.png'))]
        report.update(status='passed', finished_at=time.time(), native_xui_file_close_workflows_verified=True,
                      runtime_unchanged=True, source_inputs_unchanged=True, completed_scenarios=15,
                      completed_modes=3, executable_sha256=sha(exe))
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
    print('Native Linux Document file/close workflows passed. Actual logs: '+str(out), flush=True)


if __name__ == '__main__':
    main()
