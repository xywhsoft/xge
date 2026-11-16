"""Qualify explicit owning host sessions against real local WGL/EGL contexts.

No network, attack, hardware-performance or runtime installation tests. Graph
protocol stepping uses instrumented C owners, not a claimed language-cycle or
physical module-unload qualification.
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
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('root', 'test-root', 'engine', 'runtime', 'out'):
        p.add_argument('--' + name, type=Path, required=True)
    p.add_argument('--static', type=Path)
    a = p.parse_args()
    root, tests, engine, runtime, out = [v.resolve() for v in
        (a.root, a.test_root, a.engine, a.runtime, a.out)]
    windows = os.name == 'nt'
    sha = lambda path: hashlib.sha256(Path(path).read_bytes()).hexdigest()
    canonical = lambda path: hashlib.sha256(Path(path).read_bytes().replace(b'\r\n', b'\n')).hexdigest()
    host = json.loads((engine / 'build-report.json').read_text())
    assert host['linked'] and host['xrt_mode'] == 'host'
    assert Path(host['source_root']).resolve() == root
    assert host['xrt_host_header_sha256'] == canonical(root / 'extlibs/xrt/xrt.h')
    image = engine / host['runtime_image']
    library = engine / host['import_library'] if windows else image
    xrt = runtime / ('xlc.dll' if windows else 'libxlc.so')
    cc = Path(shutil.which('gcc')).resolve()
    cxx = Path(shutil.which('g++')).resolve()
    source = tests / 'tests/xge/xge_host_session_test.c'
    inputs = {str(v): sha(v) for v in
        (Path(__file__).resolve(), source, cc, cxx, xrt, image, library, engine / 'build-report.json')}
    for folder in ('include', 'src/xge/core', 'src/xge/render'):
        inputs.update({str(v): sha(v) for v in (root / folder).rglob('*') if v.is_file()})
    variants = [('host', engine, library)]
    if a.static:
        static = a.static.resolve()
        data = json.loads((static / 'build-report.json').read_text())
        assert data['passed'] and data['sourceUnchanged'] and data['features'] == host['features']
        assert data['xrt_header_sha256'] == host['xrt_host_header_sha256']
        variants.append(('static', static, static / data['archive']))
        for v in (static / 'build-report.json', static / data['archive']):
            inputs[str(v)] = sha(v)
    out.mkdir(parents=True, exist_ok=False)
    env = dict(os.environ)
    if windows:
        for v in (image, xrt):
            shutil.copy2(v, out / v.name)
    else:
        env['LD_LIBRARY_PATH'] = str(runtime) + ':' + str(engine) + ':' + env.get('LD_LIBRARY_PATH', '')
        env['LIBGL_ALWAYS_SOFTWARE'] = '1'
    report = dict(complete=False, passed=False, scope=__doc__, actualGraphics=True,
        hardwarePerformance=False, shippingDelivery=False, inputs=inputs, rows=[])

    def save():
        (out / 'results.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf8')

    def run(name, command):
        command = list(map(str, command))
        started = time.monotonic()
        result = subprocess.run(command, cwd=out, env=env, capture_output=True, timeout=120)
        log = out / (name + '.log')
        log.write_bytes(result.stdout + result.stderr)
        report['rows'].append(dict(name=name, command=command, exitCode=result.returncode,
            seconds=round(time.monotonic() - started, 3), logSha256=sha(log)))
        save()
        print(name, result.returncode, flush=True)
        assert result.returncode == 0, log

    for variant, folder, lib in variants:
        flags = ['-std=c11', '-Wall', '-Wextra', '-Werror', '-Wno-cast-function-type',
            '-DXGE_XRT_HOST=1', '-include', folder / 'xge_build_config.h']
        flags += ['-I' + str(v) for v in (root, root / 'include', folder, engine / 'host-xrt')]
        if variant == 'static':
            flags += ['-DXGE_TEST_STATIC=1']
        elif windows:
            flags += ['-DXGE_DLL', '-DXUI_DLL']
        systems = ['-l' + name for name in data['system_libraries']] if variant == 'static' else []
        systems += ['-lopengl32', '-lgdi32', '-luser32'] if windows else [
            '-lEGL', '-Wl,-rpath,' + str(runtime), '-Wl,-rpath,' + str(engine), '-pthread', '-ldl', '-lm']
        for opt in ('O0', 'O2'):
            binary = out / (variant + '-' + opt + ('.exe' if windows else ''))
            run(variant + '-' + opt + '-build', [cc, *flags, '-' + opt, source, lib, xrt, *systems, '-o', binary])
            for mode in range(29):
                run(variant + '-' + opt + '-mode' + str(mode), [binary, str(mode)])
    header = out / 'public.h'
    header.write_text('#include <xge.h>\n', encoding='utf8')
    for std, compiler, language in (('c11', cc, 'c'), ('c++11', cxx, 'c++'), ('c++17', cxx, 'c++')):
        run('header-' + std, [compiler, '-std=' + std, '-Wall', '-Wextra', '-Werror',
            '-I' + str(root), '-I' + str(root / 'include'), '-x', language, '-fsyntax-only', header])
    report.update(complete=True, passed=True, inputsUnchanged=all(sha(p) == h for p, h in inputs.items()))
    assert report['inputsUnchanged']
    save()


if __name__ == '__main__':
    main()
