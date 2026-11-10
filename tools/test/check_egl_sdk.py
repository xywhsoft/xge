"""Qualify matched default/enabled EGL SDKs, shared engine and static archive.

Enabled Linux uses actual Mesa EGL/GLES FBO pixels in PBuffer, Surfaceless and
native X11 window contexts. Typed ELF interposition injects failures inside the real linked
engine. This does not qualify an owning XGE renderer/context session or hardware
performance. No network tests or substitute engine translation unit are used.
"""
from pathlib import Path
import argparse
import hashlib
import json
import os
import shutil
import subprocess


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','test-root','engine','static','runtime','host-header','out'):
        p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--cleanup',action='store_true',help='qualify checked teardown and startup-rollback retry')
    a=p.parse_args();root,tests,engine,static,runtime,header,out=[v.resolve() for v in
        (a.root,a.test_root,a.engine,a.static,a.runtime,a.host_header,a.out)]
    sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest()
    canonical=lambda v:hashlib.sha256(v.read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    dynamic=json.loads((engine/'build-report.json').read_text())
    archive=json.loads((static/'build-report.json').read_text())
    assert dynamic['linked'] and dynamic['xrt_mode']=='host'
    assert archive['passed'] and archive['sourceUnchanged']
    assert dynamic['features']==archive['features']
    assert dynamic['xrt_host_header_sha256']==archive['xrt_header_sha256']==canonical(header)==canonical(root/'extlibs/xrt/xrt.h')
    enabled='XGE_ENABLE_EGL' in dynamic['features'];windows=os.name=='nt'
    assert not(enabled and windows),'No qualified Windows EGL SDK target'
    for metadata in (dynamic,archive):
        sdk=metadata['egl_sdk'];assert bool(sdk)==enabled
        if sdk:assert all(sha(v)==h for v,h in sdk['inputs'].items())
    cc=Path(shutil.which('gcc')).resolve();image=engine/dynamic['runtime_image']
    library=engine/dynamic['import_library'] if windows else image
    raw=tests/('tests/xge/xge_egl_cleanup_test.c' if a.cleanup else 'tests/xge/xge_egl_sdk_test.c');fault=tests/'tests/xge/xge_egl_fault_preload.c'
    files=[Path(__file__).resolve(),cc,raw,fault,fault.with_suffix('.h'),header,image,library,runtime,
        engine/'build-report.json',static/'build-report.json',static/archive['archive'],
        engine/'xge_build_config.h',static/'xge_build_config.h',static/'xge_static_xrt_contract.h',
        root/'include/xge.h',root/'include/xge_config.h',root/'src/xge/platform/xge_egl.c']
    files.extend(v for v in (engine/'host-xrt').iterdir() if v.is_file())
    inputs={str(v):sha(v) for v in files}
    if enabled:
        for metadata in (dynamic,archive):inputs.update(metadata['egl_sdk']['inputs'])
        for v in Path('/usr/include/GLES3').glob('*.h'):inputs[str(v)]=sha(v)
    out.mkdir(parents=True,exist_ok=False);env=dict(os.environ)
    if windows:
        for v in (runtime,image):shutil.copy2(v,out/v.name)
    else:env['LD_LIBRARY_PATH']=str(out)+':'+str(runtime.parent)+':'+str(engine)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(passed=False,scope=__doc__,eglEnabled=enabled,graphicsDevice=enabled,
        checkedCleanup=a.cleanup,hardwarePerformance=False,networkTests=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    def run(name,args):
        args=list(map(str,args));z=subprocess.run(args,cwd=out,env=env,capture_output=True,timeout=60)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        report['rows'].append(dict(name=name,command=args,exitCode=z.returncode,logSha256=sha(log)))
        save();print(name,z.returncode,flush=True);assert not z.returncode,log
    warning=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type']
    shim=[]
    if enabled:
        interpose=out/'libEglFault.so'
        run('interposer-build',[cc,*warning,'-O2','-fPIC','-shared',fault,'-ldl','-o',interpose])
        shim=[interpose,'-lEGL','-lGLESv2','-lX11']
    for variant,folder,lib in [('host',engine,library),('static',static,static/archive['archive'])]:
        flags=[*warning,'-DXGE_XRT_HOST=1','-include',folder/'xge_build_config.h']
        flags+=['-I'+str(v) for v in (root,root/'include',folder,engine/'host-xrt')]
        if variant=='static':flags+=['-DXGE_TEST_STATIC=1']
        elif windows:flags+=['-DXGE_DLL','-DXUI_DLL']
        systems=['-l'+name for name in archive['system_libraries']] if variant=='static' else []
        if not windows:systems+=['-Wl,-rpath,'+str(out),'-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine),'-pthread','-ldl','-lm']
        for opt in ('O0','O2'):
            binary=out/(variant+'-'+opt+('.exe' if windows else ''))
            run(variant+'-'+opt+'-build',[cc,*flags,'-'+opt,raw,*shim,lib,runtime,*systems,'-o',binary])
            if a.cleanup:
                cases=[(0,s) for s in range(1,5)]+[(6,s) for s in (3,4)]+[(7,s) for s in (2,3,4)] if enabled else [(0,0)]
                if enabled:cases.extend((0,s,'checked') for s in range(1,5))
            else:cases=[(m,) for m in (range(11) if enabled else (0,8))]
            for case in cases:
                run(variant+'-'+opt+'-mode'+'-'.join(map(str,case)),[binary,*map(str,case)])
    report.update(passed=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()))
    assert report['inputsUnchanged'];save()


if __name__=='__main__':main()
