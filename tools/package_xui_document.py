#!/usr/bin/env python3
"""Create a fresh standalone Windows Document package, optionally with a separate browser demo."""
import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import posixpath
import re
import shutil
import subprocess
import sys
import time
import zipfile


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def machine(path):
    data = Path(path).read_bytes()
    if len(data) < 64 or data[:2] != b'MZ':
        raise ValueError('Not a Windows executable: '+str(path))
    offset = int.from_bytes(data[60:64], 'little')
    if data[offset:offset+4] != b'PE\0\0':
        raise ValueError('Invalid PE header: '+str(path))
    return int.from_bytes(data[offset+4:offset+6], 'little')


def portable_guide(source, name, guide_roles):
    """Resolve source-relative links to the small set of bundled guides."""
    def redirect(match):
        target = match.group(1)
        if '://' in target or target.startswith(('mailto:', '#')):
            return match.group(0)
        path, separator, fragment = target.partition('#')
        resolved = (source.parent/path).resolve()
        if resolved not in guide_roles:
            raise ValueError('Guide link has no packaged destination: '+str(source)+' -> '+target)
        relative = posixpath.relpath(guide_roles[resolved], str(PurePosixPath(name).parent))
        return ']('+relative+(separator+fragment if separator else '')+')'

    return re.sub(r'\]\(([^)]+)\)', redirect, source.read_text(encoding='utf-8'))


def main():
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out-dir', type=Path, required=True, help='New package directory; also creates sibling .zip and .report.json')
    parser.add_argument('--runtime-dir', type=Path, default=root/'build')
    parser.add_argument('--document-exe', type=Path, default=root/'build/xui_document.exe')
    parser.add_argument('--webview-dir', type=Path, help='Optional WebView2 runtime containing the standalone xui_webview.exe')
    parser.add_argument('--messages-exe', type=Path, help='Optional native Document/MessageList example using the Document runtime')
    parser.add_argument('--verify', action='store_true', help='Run bundled examples from their own folders with system-only PATH')
    args = parser.parse_args()
    if sys.platform != 'win32':
        parser.error('This runnable package currently targets Windows only; Linux uses its own build/test entries.')
    destination = args.out_dir.resolve()
    archive = Path(str(destination)+'.zip')
    report_path = Path(str(destination)+'.report.json')
    if any(p.exists() for p in (destination, archive, report_path)):
        parser.error('Output directory, ZIP and report must all be new')
    runtime = args.runtime_dir.resolve()
    # One source may be needed in multiple application folders. Keep roles
    # separate even when Document and WebView use the same runtime directory.
    files = [(runtime/'xge.dll', 'document/xge.dll'), (args.document_exe.resolve(), 'document/xui_document.exe')]
    guides = [(root/'docs/XUI_DOCUMENT_RUN_PACKAGE.md', 'README.md')]
    for name in ('XUI_DOCUMENT_NATIVE_FIND.md', 'XUI_DOCUMENT_NATIVE_RESIZE.md', 'XUI_DOCUMENT_FORMAT_MENU.md'):
        guides.append((root/'docs'/name, 'docs/'+name))
    document_loader = runtime/'WebView2Loader.dll'
    if document_loader.is_file():
        files.append((document_loader, 'document/WebView2Loader.dll'))
    for p in sorted((root/'res').glob('msgbox_*.png')):
        files.append((p, 'document/res/'+p.name))
    if not (root/'res/msgbox_info.png').is_file():
        parser.error('Missing Document image-decoder probe resource')
    if args.webview_dir:
        webview = args.webview_dir.resolve()
        for name in ('xge.dll', 'WebView2Loader.dll', 'xui_webview.exe'):
            files.append((webview/name, 'webview/'+name))
        guides.append((root/'examples/xui_webview/README.md', 'webview/README.md'))
    if args.webview_dir or document_loader.is_file():
        # Preserve the loader's accompanying SDK notices when present.
        sdk = Path(os.environ.get('WEBVIEW2_SDK_DIR', root/'artifacts/xui-webview-probe/sdk'))
        for name in ('LICENSE.txt', 'NOTICE.txt'):
            if (sdk/name).is_file():
                files.append(((sdk/name).resolve(), 'notices/WebView2/'+name))
    if args.messages_exe:
        files.append((args.messages_exe.resolve(), 'document/xui_document_messages.exe'))
    # Copy existing dependency license/notice files, keeping original contents.
    for p in sorted((root/'lib').rglob('*')):
        if p.is_file() and p.name.upper().startswith(('LICENSE', 'COPYING', 'NOTICE')):
            files.append((p, 'notices/'+str(p.relative_to(root)).replace('\\', '/')))
    # These single-file dependencies carry their notices inside their headers.
    for name in ('lib/xrt/xrt.h', 'lib/sokol/sokol_app.h', 'lib/sokol/sokol_gfx.h',
                 'lib/stb/stb_image.h', 'lib/stb/stb_image_write.h', 'lib/stb/stb_truetype.h',
                 'lib/miniaudio/miniaudio.h'):
        p = root/name
        if p.is_file():
            files.append((p, 'notices/'+name))
    missing = [str(p) for p, name in files+guides if not p.is_file()]
    if missing:
        parser.error('Missing package inputs: '+', '.join(missing))
    architectures = {machine(p) for p, name in files if p.suffix.lower() in ('.exe', '.dll')}
    if len(architectures) != 1:
        parser.error('All package executables and DLLs must use the same PE architecture')
    source_inputs = {str(p): sha(p) for p, name in files+guides}
    source_inputs[str(Path(__file__).resolve())] = sha(__file__)
    destination.mkdir(parents=True)
    report = dict(status='running', started_at=time.time(), source_root=str(root), input_sha256=source_inputs,
                  pe_machine=architectures.pop(), verification_requested=args.verify, checks=[],
                  compiler_on_verification_path=False, fonts_bundled=False, browser_runtime_bundled=False,
                  webview_included=bool(args.webview_dir), physical_ime_verified=False, overall_goal_complete=False)
    report['document_messages_included'] = bool(args.messages_exe)
    report['document_webview_loader_included'] = document_loader.is_file()
    report['guides'] = []

    def save():
        report_path.write_text(json.dumps(report, indent=2), encoding='utf-8')

    save()
    try:
        for source, name in files:
            target = destination/name
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, target)
            if sha(target) != source_inputs[str(source)]:
                raise RuntimeError('Copied file changed: '+str(source))
        guide_roles = {source.resolve(): name for source, name in guides}
        for source, name in guides:
            target = destination/name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text(portable_guide(source, name, guide_roles), encoding='utf-8')
            report['guides'].append(dict(source=str(source), path=name,
                                        source_sha256=source_inputs[str(source)], packaged_sha256=sha(target)))
        # Gate the actual delivered files even when native --verify is omitted.
        report['guide_links'] = []
        for row in report['guides']:
            for target in re.findall(r'\]\(([^)]+)\)', (destination/row['path']).read_text(encoding='utf-8')):
                if '://' in target or target.startswith(('mailto:', '#')):
                    continue
                resolved = (destination/PurePosixPath(row['path']).parent/target.split('#', 1)[0]).resolve()
                if destination not in resolved.parents or not resolved.is_file():
                    raise RuntimeError('Packaged guide link is missing or outside package: '+row['path']+' -> '+target)
                report['guide_links'].append(dict(source=row['path'], target=target,
                                                  destination=resolved.relative_to(destination).as_posix()))
        report['portable_guides_verified'] = True
        # The example writes diagnostic PNGs here. Keep the directory in ZIPs,
        # even when packaging without --verify.
        (destination/'document/artifacts/xui-document-rebuild').mkdir(parents=True)
        (destination/'Open Document.cmd').write_text('@echo off\ncd /d "%~dp0document"\nxui_document.exe %*\n', encoding='utf-8')
        if args.webview_dir:
            (destination/'Open WebView.cmd').write_text('@echo off\ncd /d "%~dp0webview"\nxui_webview.exe %*\n', encoding='utf-8')
        if args.messages_exe:
            (destination/'Open Messages.cmd').write_text('@echo off\ncd /d "%~dp0document"\nxui_document_messages.exe %*\n', encoding='utf-8')
        if args.verify:
            env = dict(os.environ)
            system = Path(env.get('SystemRoot', r'C:\Windows'))
            env['PATH'] = str(system/'System32')+os.pathsep+str(system)
            for key in ('XUI_DOCUMENT_FONT', 'XUI_DOCUMENT_FONT_BOLD', 'XUI_DOCUMENT_FONT_ITALIC',
                        'XUI_DOCUMENT_FONT_MONO', 'XUI_DOCUMENT_CJK_FONT', 'XUI_WEBVIEW_FONT'):
                env.pop(key, None)
            checks = [('document', 'xui_document.exe', flag) for flag in
                      ('--verify', '--verify-async', '--verify-image-placeholder', '--verify-files')]
            if args.webview_dir:
                checks.append(('webview', 'xui_webview.exe', '--verify'))
            if args.messages_exe:
                checks.append(('document', 'xui_document_messages.exe', '--verify'))
            logs = destination/'verification'
            logs.mkdir()
            for app, exe, flag in checks:
                name = ('document-messages' if exe == 'xui_document_messages.exe' else app)+flag
                log = logs/(name+'.log')
                command = [str(destination/app/exe), flag]
                row = dict(name=name, command=command, cwd=str(destination/app), log=str(log))
                report['checks'].append(row)
                with log.open('xb') as stream:
                    result = subprocess.run(command, cwd=destination/app, env=env, stdout=stream,
                                            stderr=subprocess.STDOUT, timeout=90)
                row.update(actual_exit=result.returncode, log_sha256=sha(log))
                save()
                print(row['name']+' actual EXIT '+str(result.returncode), flush=True)
                if result.returncode:
                    raise RuntimeError('Package verification failed: '+str(log))
        if any(sha(p) != h for p, h in source_inputs.items()):
            raise RuntimeError('Package inputs changed during verification')
        payload = {str(p.relative_to(destination)).replace('\\', '/'): sha(p)
                   for p in sorted(destination.rglob('*')) if p.is_file()}
        manifest = dict(format_version=1, files_sha256=payload, verification_passed=args.verify,
                        verified_commands=len(report['checks']), pe_machine=report['pe_machine'],
                        portable_guides_verified=True, guide_paths=[row['path'] for row in report['guides']],
                        physical_ime_verified=False, all_platforms_verified=False)
        (destination/'manifest.json').write_text(json.dumps(manifest, indent=2), encoding='utf-8')
        with zipfile.ZipFile(archive, 'x', compression=zipfile.ZIP_DEFLATED) as zipped:
            for p in sorted(destination.rglob('*')):
                zipped.write(p, str(p.relative_to(destination)).replace('\\', '/'))
        report.update(status='passed', finished_at=time.time(), source_inputs_unchanged=True,
                      archive=str(archive), archive_sha256=sha(archive), archived_files=len(payload)+1,
                      manifest_sha256=sha(destination/'manifest.json'),
                      standalone_automatic_checks_passed=bool(args.verify))
        save()
    except BaseException as error:
        report.update(status='failed', finished_at=time.time(), error=str(error))
        save()
        raise
    print('Runnable package: '+str(archive), flush=True)


if __name__ == '__main__':
    main()
