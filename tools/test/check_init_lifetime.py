"""Qualify engine code retention and copied titles against one matched native build.

Uses one explicit host XRT. --graphics also verifies the actual OS window title;
it does not qualify a shipping language delivery or custom/offscreen backends.
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
    for name in ('root','test-root','engine','runtime','host-header','out'):
        p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--graphics',action='store_true')
    a=p.parse_args();root,tests,engine,runtime,header,out=[v.resolve() for v in
        (a.root,a.test_root,a.engine,a.runtime,a.host_header,a.out)]
    sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest()
    canonical=lambda v:hashlib.sha256(v.read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    metadata=json.loads((engine/'build-report.json').read_text())
    assert metadata['linked'] and metadata['xrt_mode']=='host'
    assert metadata['xrt_host_header_sha256']==canonical(header)==canonical(root/'extlibs/xrt/xrt.h')
    cc=Path(shutil.which('gcc')).resolve();image=engine/metadata['runtime_image']
    library=engine/metadata['import_library'] if os.name=='nt' else image
    source=tests/'tests/xge/xge_init_lifetime_test.c'
    files=[Path(__file__).resolve(),cc,source,header,image,library,runtime,engine/'build-report.json',
        engine/'xge_build_config.h',root/'include/xge.h',root/'src/xge/core/xge_core.c',
        root/'src/xge/core/xge_graphics_lifetime.h']
    files.extend(v for v in (engine/'host-xrt').iterdir() if v.is_file())
    inputs={str(v):sha(v) for v in files};out.mkdir(parents=True,exist_ok=False)
    env=dict(os.environ)
    if os.name=='nt':
        for v in (runtime,image):shutil.copy2(v,out/v.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+str(engine)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(passed=False,scope=__doc__,graphicsDevice=a.graphics,networkTests=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    def run(name,args):
        args=list(map(str,args));z=subprocess.run(args,cwd=out,env=env,capture_output=True,timeout=60)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        report['rows'].append(dict(name=name,command=args,exitCode=z.returncode,logSha256=sha(log)))
        save();print(name,z.returncode,flush=True);assert not z.returncode,log
    flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-DXGE_XRT_HOST=1','-include',engine/'xge_build_config.h']
    flags+=['-I'+str(v) for v in (root,root/'include',engine/'host-xrt')]
    if os.name=='nt':flags+=['-DXGE_DLL','-DXUI_DLL']
    systems=['-luser32'] if os.name=='nt' else ['-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine),'-pthread','-ldl','-lm','-lX11']
    for opt in ('O0','O2'):
        binary=out/(opt+('.exe' if os.name=='nt' else ''))
        run(opt+'-build',[cc,*flags,'-'+opt,source,library,runtime,*systems,'-o',binary])
        for mode in range(9):
            if mode==7 and not a.graphics:continue
            run(opt+'-mode'+str(mode),[binary,str(mode)])
    report.update(passed=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()))
    assert report['inputsUnchanged'];save()


if __name__=='__main__':main()
