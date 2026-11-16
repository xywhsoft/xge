"""Qualify immutable native backend references on a frozen XRT baseline.

CPU-only O0/O2 lifecycle/OOM/thread tests, public C/C++ headers and one complete
engine translation-unit syntax check. No SDK is rebuilt or installed, no
backend is selected, and no graphics/host loop is started.
"""
from pathlib import Path
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('root', 'baseline-source', 'runtime-dir', 'compiler-report',
                 'engine-report', 'out'):
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args()
    root, baseline, runtime, build, engine, out = [p.resolve() for p in
        (args.root, args.baseline_source, args.runtime_dir, args.compiler_report,
         args.engine_report, args.out)]
    assert not out.is_relative_to(root) and not out.is_relative_to(baseline)
    sha = lambda path: hashlib.sha256(Path(path).read_bytes()).hexdigest()
    win = os.name == 'nt'
    tools = {name: Path(shutil.which(name)).resolve() for name in ('gcc', 'g++')}
    metadata = json.loads(build.read_text())
    assert metadata['complete'] and metadata['passed'] and metadata['fullTccXrtAndVfs']
    assert not metadata['changedInputs']
    for name in (('xl.exe', 'xl.dll', 'xlc.dll') if win else ('xl', 'libxl.so', 'libxlc.so')):
        assert sha(runtime / name) == metadata['products'][name]['sha256']
    assert sha(baseline / 'extlibs/xrt/xrt.h') == sha(Path(metadata['source']) / 'lib/xrt/xrt.h')
    overlays = ['include/xge.h', 'src/xge/core/xge_impl.c',
                'src/xge/core/xge_backend_owned.c', 'src/xge/core/xge_backend_owned.inl',
                'tests/xge/xge_backend_owned_test.c']
    inputs = {str(root / name): sha(root / name) for name in overlays}
    for path in (Path(__file__), build, engine, *tools.values(),
                 runtime / ('xlc.dll' if win else 'libxlc.so')):
        inputs[str(path)] = sha(path)
    out.mkdir(parents=True, exist_ok=False)
    source = out / 'source'
    # Only compiler inputs: copying apps, screenshots and build artifacts is
    # unrelated to this gate and particularly expensive across WSL mounts.
    source.mkdir()
    for directory in ('include', 'src', 'extlibs', 'modules', 'single'):
        shutil.copytree(baseline / directory, source / directory,
                        ignore=shutil.ignore_patterns('build', '__pycache__', '*.pyc'))
    for name in overlays:
        target = source / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(root / name, target)
    frozen = {str(p): sha(p) for p in source.rglob('*') if p.is_file()}
    report = dict(complete=False, passed=False, scope=__doc__, nativeSdkRebuilt=False,
                  shippingDelivery=False, inputs=inputs, frozenSource=frozen, rows=[])
    env = dict(os.environ)
    key = 'PATH' if win else 'LD_LIBRARY_PATH'
    env[key] = str(runtime) + os.pathsep + env.get(key, '')

    def save():
        temporary = out / 'results.json.tmp'
        temporary.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
        os.replace(temporary, out / 'results.json')

    def run(name, command, marker=None):
        command = list(map(str, command))
        start = time.monotonic()
        result = subprocess.run(command, cwd=source, env=env, capture_output=True, timeout=300)
        log = out / (name + '.log')
        data = result.stdout + result.stderr
        log.write_bytes(data)
        report['rows'].append(dict(name=name, command=command, exitCode=result.returncode,
                                  seconds=time.monotonic() - start, logSha256=sha(log)))
        save()
        print(name, result.returncode, flush=True)
        assert not result.returncode, log
        if marker:
            assert marker in data, log

    try:
        flags = ['-std=c11', '-Wall', '-Wextra', '-Werror', '-Wno-unused-function',
                 '-DXGE_STATIC_BUILD=1', '-DXRT_USE_SHARED=1', '-DXRT_MODULE_ALL',
                 '-I' + str(source), '-I' + str(source / 'include'),
                 '-I' + str(source / 'src/xge/core')]
        library = runtime / ('xlc.dll' if win else 'libxlc.so')
        for optimization in ('O0', 'O2'):
            binary = out / ('backend-' + optimization + ('.exe' if win else ''))
            run(optimization + '-build', [tools['gcc'], *flags, '-' + optimization,
                source / 'src/xge/core/xge_backend_owned.c',
                source / 'tests/xge/xge_backend_owned_test.c', library, '-o', binary])
            run(optimization + '-run', [binary], b'BACKEND_OWNED_CPU_OK')
        public = out / 'public.h'
        public.write_text('#include "xge.h"\n', encoding='utf-8')
        for standard, cc, language in (('c11', 'gcc', 'c'), ('c++11', 'g++', 'c++'),
                                       ('c++17', 'g++', 'c++')):
            run('header-' + standard, [tools[cc], '-std=' + standard, '-Wall', '-Wextra',
                '-Werror', '-DXGE_STATIC_BUILD=1', '-I' + str(source),
                '-I' + str(source / 'include'), '-x', language, '-fsyntax-only', public])
        # These are the already-qualified profile's feature/include flags only.
        # The added public header changes its ABI fingerprint: this syntax gate
        # neither produces a new SDK nor reuses the old SDK's ABI acceptance.
        profile = json.loads(engine.read_text())
        assert profile['linked'] and profile['xrt_mode'] == 'host'
        run('engine-integration-syntax', [tools['gcc'], *profile['flags'],
                                         '-fsyntax-only', source / 'single/xge.c'])
        report.update(complete=True, passed=True,
                      inputsUnchanged=all(sha(p) == h for p, h in inputs.items()),
                      sourceUnchanged=all(sha(p) == h for p, h in frozen.items()))
        assert report['inputsUnchanged'] and report['sourceUnchanged']
    except BaseException as error:
        report.update(complete=True, error=repr(error))
        raise
    finally:
        save()


if __name__ == '__main__':
    main()
