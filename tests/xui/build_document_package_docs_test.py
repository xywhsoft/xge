#!/usr/bin/env python3
"""Check the runnable ZIP's actual guide routes without a source checkout."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import posixpath
import re
import zipfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archive', type=Path, required=True)
    parser.add_argument('--out-dir', type=Path, required=True)
    args = parser.parse_args()
    if args.out_dir.exists():
        parser.error('--out-dir must be new so earlier results remain intact')
    args.out_dir.mkdir(parents=True)
    report = dict(status='running', archive=str(args.archive.resolve()), checks=[],
                  archive_sha256=hashlib.sha256(args.archive.read_bytes()).hexdigest(),
                  source_checkout_used_for_routes=False)
    with zipfile.ZipFile(args.archive) as archive:
        names = set(archive.namelist())
        expected = ['README.md', 'docs/validation/xui-document/XUI_DOCUMENT_NATIVE_FIND.md',
                    'docs/validation/xui-document/XUI_DOCUMENT_NATIVE_RESIZE.md', 'docs/validation/xui-document/XUI_DOCUMENT_FORMAT_MENU.md']
        if 'Open WebView.cmd' in names:
            expected.append('webview/README.md')
        for path in expected:
            report['checks'].append(dict(name='guide-present', path=path, passed=path in names))
        links = 0
        for path in sorted(name for name in names if name in expected):
            text = archive.read(path).decode('utf-8')
            for target in re.findall(r'\]\(([^)]+)\)', text):
                if '://' in target or target.startswith(('mailto:', '#')):
                    continue
                destination = posixpath.normpath(str(PurePosixPath(path).parent/target.split('#', 1)[0]))
                contained = not destination.startswith('../') and not PurePosixPath(destination).is_absolute()
                report['checks'].append(dict(name='guide-link', source=path, target=target,
                                             destination=destination, passed=contained and destination in names))
                links += 1
            if path == 'webview/README.md':
                report['checks'].append(dict(name='browser-back-to-main-guide', passed='](../README.md)' in text))
            if path == 'docs/validation/xui-document/XUI_DOCUMENT_NATIVE_FIND.md':
                report['checks'].append(dict(name='find-back-to-main-guide', passed='](../README.md)' in text))
        report['local_links_checked'] = links
        report['status'] = 'passed' if all(row['passed'] for row in report['checks']) and links else 'failed'
    (args.out_dir/'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    print('Runnable documentation routes:', report['status'], ';', links, 'actual local links checked.')
    if report['status'] != 'passed':
        for row in report['checks']:
            if not row['passed']:
                print(row)
        raise SystemExit(1)


if __name__ == '__main__':
    main()
