"""Matched owning EGL/XGE renderer and graph/lifetime tests, shared and static.

Enabled Linux executes the actual XGE renderer, not just raw GLES FBO calls.
Default SDKs prove explicit unsupported admission with no native allocation.
No network or hardware-performance tests are performed.
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
    p.add_argument('--faults',action='store_true',help='typed real EGL/renderer failure injection (enabled Linux)')
    a=p.parse_args();root,tests,engine,static,runtime,header,out=[v.resolve() for v in
        (a.root,a.test_root,a.engine,a.static,a.runtime,a.host_header,a.out)]
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    canonical=lambda p:hashlib.sha256(Path(p).read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    host=json.loads((engine/'build-report.json').read_text());archive=json.loads((static/'build-report.json').read_text())
    assert host['linked'] and host['xrt_mode']=='host' and archive['passed'] and archive['sourceUnchanged']
    assert host['features']==archive['features'] and host['xrt_host_header_sha256']==archive['xrt_header_sha256']==canonical(header)==canonical(root/'extlibs/xrt/xrt.h')
    enabled='XGE_ENABLE_EGL' in host['features'];windows=os.name=='nt';assert not(enabled and windows)
    cc=Path(shutil.which('gcc')).resolve();image=engine/host['runtime_image'];library=engine/host['import_library'] if windows else image
    assert not a.faults or enabled
    source=tests/('tests/xge/xge_graphics_context_fault_test.c' if a.faults else 'tests/xge/xge_graphics_context_test.c')
    files=[Path(__file__).resolve(),source,cc,runtime,image,library,header,engine/'build-report.json',
        static/'build-report.json',static/archive['archive'],engine/'xge_build_config.h',static/'xge_build_config.h',static/'xge_static_xrt_contract.h']
    files.extend((root/'include').glob('*.h'));files.extend(v for v in (engine/'host-xrt').iterdir() if v.is_file())
    for folder in ('core','render','platform'):files.extend(v for v in (root/'src/xge'/folder).rglob('*') if v.is_file())
    if enabled:
        for value in (host,archive):files.extend(Path(v) for v in value['egl_sdk']['inputs'])
    if a.faults:files.extend(tests/('tests/xge/xge_egl_fault_preload'+v) for v in ('.c','.h'))
    inputs={str(v):sha(v) for v in files};out.mkdir(parents=True,exist_ok=False);env=dict(os.environ)
    if windows:
        for v in (image,runtime):shutil.copy2(v,out/v.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+str(engine)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(passed=False,scope=__doc__,eglEnabled=enabled,actualXgeRenderer=enabled,
        hardwarePerformance=False,networkTests=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    def run(name,args):
        args=list(map(str,args));r=subprocess.run(args,cwd=out,env=env,capture_output=True,timeout=90)
        log=out/(name+'.log');log.write_bytes(r.stdout+r.stderr)
        report['rows'].append(dict(name=name,command=args,exitCode=r.returncode,logSha256=sha(log)))
        save();print(name,r.returncode,flush=True);assert not r.returncode,log
    shim=[]
    if a.faults:
        interposer=out/'libEglFault.so'
        run('interposer-build',[cc,'-std=c11','-O2','-Wall','-Wextra','-Werror','-fPIC','-shared',
            tests/'tests/xge/xge_egl_fault_preload.c','-ldl','-o',interposer])
        shim=[interposer]
    for variant,folder,lib in [('host',engine,library),('static',static,static/archive['archive'])]:
        flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-DXGE_XRT_HOST=1','-include',folder/'xge_build_config.h']
        flags+=['-I'+str(v) for v in (root,root/'include',folder,engine/'host-xrt')]
        if variant=='static':flags+=['-DXGE_TEST_STATIC=1']
        elif windows:flags+=['-DXGE_DLL','-DXUI_DLL']
        systems=['-l'+name for name in archive['system_libraries']] if variant=='static' else []
        if enabled:systems+=['-lEGL']
        if not windows:systems+=['-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine),'-pthread','-ldl','-lm']
        for opt in ('O0','O2'):
            binary=out/(variant+'-'+opt+('.exe' if windows else ''))
            run(variant+'-'+opt+'-build',[cc,*flags,'-'+opt,source,*shim,lib,runtime,*systems,'-o',binary])
            for mode in (range(18) if a.faults else range(14) if enabled else (0,13)):
                run(variant+'-'+opt+'-mode'+str(mode),[binary,str(mode)])
    report.update(passed=True,inputsUnchanged=all(sha(p)==h for p,h in inputs.items()))
    assert report['inputsUnchanged'];save()


if __name__=='__main__':main()
