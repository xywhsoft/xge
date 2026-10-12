#!/usr/bin/env python3
"""Test native Document text clipboard on an owned, authenticated Xvfb display."""
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
    parser.add_argument('--out-dir', type=Path, help='New directory for executables, logs and report')
    parser.add_argument('--runtime-dir', type=Path, help='Existing normal/default Linux runtime; otherwise build one')
    args = parser.parse_args()
    if not sys.platform.startswith('linux'):
        parser.error('Run on Linux/X11 (including WSL); Windows uses the native clipboard regression entry.')
    out = (args.out_dir or root / 'build' / ('document-x11-clipboard-' + datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%SZ') + '-' + str(os.getpid()))).resolve()
    if out.exists():
        parser.error('--out-dir must be new so earlier evidence remains intact')
    out.mkdir(parents=True)
    runtime = args.runtime_dir.resolve() if args.runtime_dir else out / 'runtime'
    auth = out / 'Xauthority'
    server = None
    env = dict(os.environ, LC_ALL='C.UTF-8', LANG='C.UTF-8', LIBGL_ALWAYS_SOFTWARE='1',
               GALLIUM_DRIVER='llvmpipe', LP_NUM_THREADS='1', XAUTHORITY=str(auth), XUI_TEST_PRIVATE_DISPLAY='1')
    report = dict(status='running', source_root=str(root), started_at=time.time(), runtime=str(runtime),
                  checks=[], user_display_accessed=False, user_clipboard_accessed=False,
                  native_physical_gpu_verified=False, physical_ime_verified=False,
                  advanced_matrix_requested=False, overall_design_complete=False)

    def save():
        (out / 'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')

    def check(name, command, timeout=90):
        command = list(map(str, command))
        log = out / (name + '.log')
        row = dict(name=name, command=command, log=str(log))
        report['checks'].append(row)
        started = time.monotonic()
        with log.open('xb') as stream:
            child = subprocess.Popen(command, cwd=root, env=env, stdout=stream, stderr=subprocess.STDOUT,
                                     start_new_session=True)
            try:
                result = child.wait(timeout=timeout)
            except subprocess.TimeoutExpired:
                # Only this newly created process group is owned by the driver.
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
        print(name + ' actual EXIT ' + str(result), flush=True)
        if result:
            print(log.read_text(encoding='utf-8', errors='replace')[-6000:], flush=True)
            raise RuntimeError(name + ' failed; see ' + str(log))
        return log

    save()
    try:
        if not args.runtime_dir:
            check('normal-build', [sys.executable, root/'tools/build/build.py', 'default', runtime], 900)
            build = json.loads((runtime/'build-report.json').read_text())
            report.update(compiled_units=len(build['compiled']), reused_units=len(build['reused']))
        library = runtime / 'libxge.so'
        config = runtime / 'xge_build_config.h'
        if not library.is_file() or not config.is_file():
            raise RuntimeError('Runtime must contain libxge.so and xge_build_config.h from the normal builder')
        library_hash, config_hash = sha(library), sha(config)
        report.update(runtime_sha256=library_hash, config_sha256=config_hash)
        check('runtime-dependencies', ['ldd', library])
        exe = out / 'xui_document_x11_clipboard_test'
        check('compile', ['gcc', '-std=c11', '-O2', '-g', '-Wall', '-Wextra', '-Werror', '@config/includes.rsp', '-DXGE_DLL', '-DXUI_DLL',
                          '-o', exe, 'tests/xui/xui_document_x11_clipboard_test.c', 'tests/xui/xui_test_proxy.c',
                          '-L'+str(runtime), '-Wl,-rpath,'+str(runtime), '-lxge', '-lX11', '-lm'])
        dependencies = check('test-dependencies', ['ldd', exe]).read_text()
        if str(library) not in dependencies or 'not found' in dependencies:
            raise RuntimeError('Test did not resolve the requested normal runtime')
        cookie = secrets.token_hex(16)
        subprocess.run(['xauth', '-f', str(auth), 'add', ':0', '.', cookie], check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        rd, wr = os.pipe()
        try:
            with (out/'Xvfb.log').open('xb') as stream:
                server = subprocess.Popen(['Xvfb', '-displayfd', str(wr), '-screen', '0', '128x128x24',
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
        env['DISPLAY'] = ':' + number
        if env['DISPLAY'] == os.environ.get('DISPLAY'):
            raise RuntimeError('Refusing to use the invoking display')
        subprocess.run(['xauth', '-f', str(auth), 'add', env['DISPLAY'], '.', cookie], check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        report['private_display'] = env['DISPLAY']
        check('native-run', [exe, '--isolated-display'])
        if sha(library) != library_hash or sha(config) != config_hash:
            raise RuntimeError('Runtime or configuration changed during validation')
        report.update(status='passed', finished_at=time.time(), native_x11_text_clipboard_verified=True,
                      runtime_unchanged=True, completed_cases=34, executable_sha256=sha(exe))
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
    print('Native X11 Document text clipboard passed. Actual logs: ' + str(out), flush=True)


if __name__ == '__main__':
    main()
