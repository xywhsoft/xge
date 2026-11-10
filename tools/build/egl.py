"""Resolve the optional system EGL SDK before compiling engine translation units.

The qualified build target is Linux with the pkg-config EGL system SDK. Driver
libraries remain system dependencies, not copied runtime payloads. Other targets
fail explicitly until their SDK/link/runtime deployment has been qualified.
"""
from pathlib import Path
import hashlib
import os
import shlex
import shutil
import subprocess


def configure(features, windows, compiler):
    if 'XGE_ENABLE_EGL' not in features:
        return dict(cflags=[], libraries=[], metadata=None)
    if windows:
        raise RuntimeError('XGE_ENABLE_EGL=1 has no qualified Windows EGL SDK; '
                           'use the normal Windows OpenGL SDK. Windows EGL SDK '
                           'and runtime deployment remain a separate target.')
    pkg=shutil.which('pkg-config')
    if not pkg:
        raise RuntimeError('XGE_ENABLE_EGL=1 requires pkg-config and the system EGL development SDK')
    def query(*args):
        result=subprocess.run([pkg,*args,'egl'],capture_output=True,text=True)
        if result.returncode:
            raise RuntimeError('System EGL SDK lookup failed: '+result.stderr.strip())
        return result.stdout.strip()
    cflags=shlex.split(query('--cflags'))
    other=shlex.split(query('--libs-only-L','--libs-only-other'))
    if other:
        raise RuntimeError('Non-system EGL linker paths/options require separate SDK deployment qualification: '+str(other))
    libraries=[];inputs={}
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    inputs[str(Path(__file__).resolve())]=sha(__file__)
    for flag in shlex.split(query('--libs-only-l')):
        if not flag.startswith('-l') or len(flag)==2:
            raise RuntimeError('Unsupported system EGL library spelling: '+flag)
        name=flag[2:];libraries.append(name)
        result=subprocess.run([compiler,'-print-file-name=lib'+name+'.so'],capture_output=True,text=True,check=True)
        path=Path(os.path.abspath(result.stdout.strip()))
        if not path.is_file():
            raise RuntimeError('Compiler cannot resolve system EGL link input: '+name)
        inputs[str(path)]=sha(path)
        inputs[str(path.resolve())]=sha(path)
    if not libraries:
        raise RuntimeError('System EGL SDK declares no link libraries')
    includedir=Path(query('--variable=includedir'))
    for name in ('EGL/egl.h','EGL/eglplatform.h','KHR/khrplatform.h'):
        path=(includedir/name).resolve()
        if not path.is_file():raise RuntimeError('Incomplete system EGL SDK: '+str(path))
        inputs[str(path)]=sha(path)
    pc=(Path(query('--variable=pcfiledir'))/'egl.pc').resolve()
    if not pc.is_file():raise RuntimeError('System EGL pkg-config provenance is missing')
    inputs[str(pc)]=sha(pc);inputs[str(Path(pkg).resolve())]=sha(pkg)
    return dict(cflags=cflags,libraries=libraries,metadata=dict(
        provider='system-pkg-config',version=query('--modversion'),
        inputs=inputs,systemLibraries=libraries,
        deployment='System EGL dispatcher and graphics driver must remain installed; no driver bytes are bundled'))


def verify(sdk):
    metadata=sdk['metadata']
    if metadata and any(not Path(path).is_file() or
            hashlib.sha256(Path(path).read_bytes()).hexdigest()!=digest
            for path,digest in metadata['inputs'].items()):
        raise RuntimeError('System EGL SDK input changed during the build')
