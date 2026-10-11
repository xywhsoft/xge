#!/usr/bin/env python3
"""Build and test the Linux Document foundation without the advanced matrix."""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import time


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    root = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out-dir', type=Path, help='New directory for executables, logs and report')
    parser.add_argument('--runtime-dir', type=Path, help='Use an existing normal/default libxge.so; otherwise build one')
    parser.add_argument('--with-x11-clipboard', action='store_true', help='Also validate native text clipboard on a private Xvfb display')
    parser.add_argument('--with-native-workflows', action='store_true', help='Also validate native XUI file/close dialogs on a private Xvfb display')
    parser.add_argument('--with-native-input', action='store_true', help='Extend --with-native-workflows with queued X11 input and window-close tests')
    parser.add_argument('--with-native-messages', action='store_true', help='Also validate native Document messages on a private Xvfb display')
    parser.add_argument('--with-native-find', action='store_true', help='Also validate native Find/Replace workflows on a private Xvfb display')
    parser.add_argument('--with-native-resize', action='store_true', help='Also resize native Document windows on a private Xvfb display')
    args = parser.parse_args()
    if args.with_native_input and not args.with_native_workflows:
        parser.error('--with-native-input requires --with-native-workflows')
    if not sys.platform.startswith('linux'):
        parser.error('Run this foundation suite on Linux (including WSL). Use the Windows basic suite for native UI.')
    out = (args.out_dir or root / 'build' / ('document-linux-basic-' + datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%SZ') + '-' + str(os.getpid()))).resolve()
    if out.exists():
        parser.error('--out-dir must be new so earlier results remain intact')
    out.mkdir(parents=True)
    runtime = args.runtime_dir.resolve() if args.runtime_dir else out / 'runtime'
    env = dict(os.environ, LC_ALL='C.UTF-8', LANG='C.UTF-8')
    report = dict(status='running', source_root=str(root), started_at=time.time(), runtime=str(runtime),
                  checks=[], advanced_matrix_requested=False, native_linux_gpu_verified=False,
                  physical_ime_verified=False, native_clipboard_verified=False, webview_linux_verified=False,
                  all_platforms_verified=False, overall_design_complete=False, x11_clipboard_requested=args.with_x11_clipboard)
    report.update(native_workflows_requested=args.with_native_workflows, native_xui_workflows_verified=False)
    report.update(native_x11_input_requested=args.with_native_input, native_x11_document_input_verified=False)
    report.update(native_messages_requested=args.with_native_messages, native_document_messages_verified=False)
    report.update(native_find_requested=args.with_native_find, native_find_replace_verified=False)
    report.update(native_resize_requested=args.with_native_resize, native_window_resize_verified=False)
    def save():
        (out / 'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    def check(name, command, timeout=900):
        command = list(map(str, command))
        log = out / (name + '.log')
        started = time.monotonic()
        row = dict(name=name, command=command, log=str(log))
        report['checks'].append(row)
        with log.open('xb') as stream:
            try:
                result = subprocess.run(command, cwd=root, env=env, stdout=stream, stderr=subprocess.STDOUT, timeout=timeout)
            except subprocess.TimeoutExpired:
                row.update(status='timed_out', log_sha256=sha(log))
                save()
                raise
        row.update(actual_exit=result.returncode, seconds=time.monotonic()-started, log_sha256=sha(log))
        save()
        print(name + ' actual EXIT ' + str(result.returncode), flush=True)
        if result.returncode:
            print(log.read_text(encoding='utf-8', errors='replace')[-6000:], flush=True)
            raise RuntimeError(name + ' failed; see ' + str(log))
        return log
    save()
    try:
        # Original core fixtures save native files here, just as the Windows
        # core build entry does. Executables and logs still use the new out dir.
        (root/'build/document').mkdir(parents=True, exist_ok=True)
        if not args.runtime_dir:
            check('normal-build', [sys.executable, root/'tools/build_profile.py', 'default', runtime])
            build = json.loads((runtime/'build-report.json').read_text())
            report.update(compiled_units=len(build['compiled']), reused_units=len(build['reused']))
        library = runtime / 'libxge.so'
        if not library.is_file() or not (runtime/'xge_build_config.h').is_file():
            raise RuntimeError('Runtime directory must contain libxge.so and xge_build_config.h from the normal builder')
        runtime_hash = sha(library)
        config_hash = sha(runtime/'xge_build_config.h')
        report.update(runtime_sha256=runtime_hash, config_sha256=config_hash)
        check('runtime-dependencies', ['ldd', library])
        common = ['gcc', '-std=c11', '-O2', '-g', '-Wall', '-Wextra', '-Werror', '-I.', '-DXGE_DLL', '-DXUI_DLL']
        link = ['-L'+str(runtime), '-Wl,-rpath,'+str(runtime), '-lxge', '-lm']
        tests = [
            ('core', ['test_xui/xui_document_test.c'], []),
            ('renderer', ['test_xui/xui_document_renderer_test.c', 'test_xui/xui_test_proxy.c'], ['-Wno-unused-function']),
            ('editor', ['test_xui/xui_document_editor_test.c', 'test_xui/xui_test_proxy.c', 'src/xui_unicode.c'], ['-Wno-unused-function']),
            ('format-commands', ['test_xui/xui_document_format_command_test.c', 'test_xui/xui_test_proxy.c'], []),
            ('boundary-deletion', ['test_xui/xui_document_boundary_delete_test.c', 'test_xui/xui_test_proxy.c'], []),
            ('text-ranges', ['test_xui/xui_document_accessible_text_test.c', 'test_xui/xui_test_proxy.c', 'src/xui_unicode.c'], ['-Wno-unused-function', '-Wno-unused-parameter']),
            ('message-list', ['test_xui/xui_message_document_test.c', 'test_xui/xui_test_proxy.c'], []),
        ]
        for name, sources, flags in tests:
            exe = out / name
            check(name+'-compile', common+flags+['-o', exe]+sources+link)
            dependencies = check(name+'-dependencies', ['ldd', exe]).read_text()
            if str(library) not in dependencies or 'not found' in dependencies:
                raise RuntimeError(name + ' did not resolve the requested runtime')
            check(name+'-run', [exe])
        # Use the existing shared core manifest and private-index corpus driver.
        # Its separate large-scale benchmarks remain in the advanced suite.
        sources = []
        for line in (root/'xui_document_sources.bat').read_text().splitlines():
            if line.startswith('set XUI_DOCUMENT_SRC='):
                sources += [p.replace('\\', '/') for p in line.split('=', 1)[1].split() if p != '%XUI_DOCUMENT_SRC%']
        if not sources:
            raise RuntimeError('Document core source manifest is empty')
        corpus = root/'artifacts/xui-document-rebuild/commonmark-0.31.2.json'
        if not corpus.is_file():
            raise RuntimeError('Missing CommonMark corpus: ' + str(corpus))
        standalone = ['gcc', '-std=c11', '-O2', '-g', '-Wall', '-Wextra', '-Werror', '-I.']
        xrt = out / 'xrt.o'
        check('corpus-xrt-compile', standalone+['-c', 'test_xui/xui_test_xrt_impl.c', '-o', xrt])
        exe = out / 'corpus'
        check('corpus-compile', standalone+['-o', exe, 'test_xui/xui_document_corpus_test.c']+sources+[xrt, '-lm'])
        check('corpus-run', [exe, '--corpus-only', corpus])
        if args.with_x11_clipboard:
            check('native-x11-clipboard', [sys.executable, root/'test_xui/build_document_x11_clipboard_test.py',
                                          '--runtime-dir', runtime, '--out-dir', out/'native-x11-clipboard'], 180)
            native = json.loads((out/'native-x11-clipboard/report.json').read_text())
            if native['status'] != 'passed' or not native['private_server_terminal'] or not native['authentication_file_removed']:
                raise RuntimeError('Native clipboard driver did not finish cleanly')
            report['native_clipboard_verified'] = True
        if args.with_native_workflows:
            command = [sys.executable, root/'test_xui/build_document_linux_workflow_test.py',
                       '--runtime-dir', runtime, '--out-dir', out/'native-xui-workflows']
            if args.with_native_input:
                command.append('--with-native-input')
            check('native-xui-workflows', command)
            native = json.loads((out/'native-xui-workflows/report.json').read_text())
            if (native['status'] != 'passed' or native['completed_scenarios'] != 15 or
                    native['completed_modes'] != 3 or not native['private_server_terminal'] or
                    not native['authentication_file_removed']):
                raise RuntimeError('Native file/close workflow driver did not finish cleanly')
            report['native_xui_workflows_verified'] = True
            if args.with_native_input:
                if not native['native_x11_document_input_verified'] or native['completed_native_input_modes'] != 3:
                    raise RuntimeError('Native X11 input tests did not pass in all three Markdown modes')
                report['native_x11_document_input_verified'] = True
        if args.with_native_messages:
            check('native-document-messages', [sys.executable, root/'test_xui/build_document_messages_test.py',
                                              '--runtime-dir', runtime, '--out-dir', out/'native-messages'], 180)
            native = json.loads((out/'native-messages/report.json').read_text())
            if (native['status'] != 'passed' or not native['private_server_terminal'] or
                    not native['authentication_file_removed'] or not native['native_renderer_verified']):
                raise RuntimeError('Native Document message driver did not finish cleanly')
            report['native_document_messages_verified'] = True
        if args.with_native_find:
            check('native-find-replace', [sys.executable, root/'test_xui/build_document_demo_find_test.py',
                                         '--runtime-dir', runtime, '--out-dir', out/'native-find', '--with-format'], 180)
            native = json.loads((out/'native-find/report.json').read_text())
            if (native['status'] != 'passed' or not native['private_server_terminal'] or
                    not native['authentication_file_removed'] or native['completed_modes'] != 4 or
                    not native['format_menu_verified']):
                raise RuntimeError('Native Find/Replace driver did not finish cleanly')
            report['native_find_replace_verified'] = True
        if args.with_native_resize:
            check('native-window-resize', [sys.executable, root/'test_xui/build_document_demo_resize_test.py',
                                          '--runtime-dir', runtime, '--out-dir', out/'native-resize', '--with-find'], 180)
            native = json.loads((out/'native-resize/report.json').read_text())
            if (native['status'] != 'passed' or not native['private_server_terminal'] or
                    not native['authentication_file_removed'] or native['completed_modes'] != 4 or
                    not native['native_window_resize_verified'] or not native['native_window_pointer_queue_verified'] or
                    not native['find_resize_verified']):
                raise RuntimeError('Native window resize driver did not finish cleanly')
            report['native_window_resize_verified'] = True
        if sha(library) != runtime_hash or sha(runtime/'xge_build_config.h') != config_hash:
            raise RuntimeError('Runtime or shared configuration changed during validation')
        report.update(status='passed', finished_at=time.time(), automatic_foundation_passed=True,
                      runtime_unchanged=True, platform_ui_acceptance_pending=True)
        save()
    except BaseException as error:
        report.update(status='failed', finished_at=time.time(), error=str(error))
        save()
        raise
    print('Linux Document foundation passed. Actual logs: ' + str(out), flush=True)


if __name__ == '__main__':
    main()
