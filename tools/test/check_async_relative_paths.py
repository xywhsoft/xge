"""Qualify relative worker reads with executable and working directories apart.

The first application-directory probe must fail; a successful CWD fallback must
not publish that speculative diagnostic. A real missing file remains a failure.
Only local 1x1 image files are used; no graphics device or network.
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
    for name in ('root','test-root','engine','runtime','host-header','out'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();root,tests,engine,runtime,header,out=[v.resolve() for v in
        (a.root,a.test_root,a.engine,a.runtime,a.host_header,a.out)]
    metadata=json.loads((engine/'build-report.json').read_text())
    assert metadata['linked'] and metadata['xrt_mode']=='host'
    sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest()
    canonical=lambda v:hashlib.sha256(v.read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    assert metadata['xrt_host_header_sha256']==canonical(header)==canonical(root/'extlibs/xrt/xrt.h')
    cc=Path(shutil.which('gcc')).resolve();image=engine/metadata['runtime_image']
    library=engine/metadata['import_library'] if os.name=='nt' else image
    source=tests/'tests/xge/xge_async_relative_path_test.c'
    files=[Path(__file__).resolve(),cc,source,header,image,library,runtime,engine/'build-report.json',
        engine/'xge_build_config.h',root/'include/xge.h',root/'src/xge/core/xge_impl.c']
    files.extend(v for v in (engine/'host-xrt').iterdir() if v.is_file())
    inputs={str(v):sha(v) for v in files};out.mkdir(parents=True,exist_ok=False)
    working=out/'working';working.mkdir()
    env=dict(os.environ)
    if os.name=='nt':
        for v in (runtime,image):shutil.copy2(v,out/v.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+str(engine)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(passed=False,scope=__doc__,networkTests=False,graphicsDevice=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    def run(name,command):
        command=list(map(str,command));r=subprocess.run(command,cwd=working,env=env,capture_output=True,timeout=30)
        log=out/(name+'.log');log.write_bytes(r.stdout+r.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=r.returncode,logSha256=sha(log)))
        save();print(name,r.returncode,flush=True);return r.returncode
    flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-DXGE_XRT_HOST=1','-include',engine/'xge_build_config.h']
    flags+=['-I'+str(v) for v in (root,root/'include',engine/'host-xrt')]
    flags+=['-DXGE_DLL','-DXUI_DLL'] if os.name=='nt' else []
    systems=[] if os.name=='nt' else ['-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine),'-pthread','-ldl','-lm']
    for opt in ('O0','O2'):
        binary=out/(opt+('.exe' if os.name=='nt' else ''))
        assert not run(opt+'-build',[cc,*flags,'-'+opt,source,library,runtime,*systems,'-o',binary])
        for missing in (0,1):
            filename='cwd-only-'+opt+'-'+str(missing)+'.tga';assert not (out/filename).exists()
            run(opt+'-missing-'+str(missing),[binary,missing,filename])
    report.update(passed=all(not v['exitCode'] for v in report['rows']),inputsUnchanged=all(sha(v)==h for v,h in inputs.items()))
    save();assert report['passed'] and report['inputsUnchanged']


if __name__=='__main__':main()
