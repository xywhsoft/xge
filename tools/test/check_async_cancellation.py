"""Qualify raw async cancellation and callback self-release on local images.

No device/window/network. This is native C contract acceptance, not the
pending managed request/target/callback owning graph or xlang delivery.
"""
from pathlib import Path
import argparse
import hashlib
import json
import os
import shutil
import subprocess


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('root','engine','runtime','host-header','out'):
        parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--test-root',type=Path)
    parser.add_argument('--owned',action='store_true',help='Check owning requests, private results, callback/code graph and deferred physical worker retirement')
    a=parser.parse_args()
    root,engine,runtime,header,out=[p.resolve() for p in (a.root,a.engine,a.runtime,a.host_header,a.out)]
    tests=a.test_root.resolve() if a.test_root else root
    metadata=json.loads((engine/'build-report.json').read_text())
    assert metadata['linked'] and metadata['xrt_mode']=='host'
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    canonical=lambda p:hashlib.sha256(p.read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    assert metadata['xrt_host_header_sha256']==canonical(header)==canonical(root/'extlibs/xrt/xrt.h')
    cc=Path(shutil.which('gcc')).resolve()
    image=engine/metadata['runtime_image']
    library=engine/metadata['import_library'] if os.name=='nt' else image
    source=tests/'tests/xge'/('xge_async_owned_test.c' if a.owned else 'xge_async_contract_test.c')
    files=[Path(__file__).resolve(),cc,source,header,image,library,runtime,
        engine/'build-report.json',engine/'xge_build_config.h',root/'include/xge.h',
        root/'src/xge/resource/xge_async.c']
    if a.owned:files.extend([root/'src/xge/resource/xge_async_owned.c',root/'src/xge/core/xge_data_lifetime.inl'])
    files.extend(p for p in (engine/'host-xrt').iterdir() if p.is_file())
    inputs={str(p):sha(p) for p in files}
    out.mkdir(parents=True,exist_ok=False)
    env=os.environ.copy()
    if os.name=='nt':
        for p in (runtime,image):shutil.copy2(p,out/p.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+str(engine)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(passed=False,scope=__doc__,ownedRequests=a.owned,networkTests=False,graphicsDevice=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    def run(name,command):
        command=list(map(str,command))
        result=subprocess.run(command,cwd=out,env=env,capture_output=True,timeout=90)
        log=out/(name+'.log');log.write_bytes(result.stdout+result.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=result.returncode,logSha256=sha(log)))
        save();print(name,result.returncode,flush=True)
        return result.returncode
    flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type',
        '-DXGE_XRT_HOST=1','-include',engine/'xge_build_config.h']
    flags+=['-I'+str(p) for p in (root,root/'include',engine/'host-xrt')]
    flags+=['-DXGE_DLL','-DXUI_DLL'] if os.name=='nt' else []
    systems=[] if os.name=='nt' else ['-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine),'-pthread','-ldl','-lm']
    for optimization in ('O0','O2'):
        binary=out/(optimization+('.exe' if os.name=='nt' else ''))
        assert not run(optimization+'-build',[cc,*flags,'-'+optimization,source,library,runtime,*systems,'-o',binary])
        for mode in range(1,24 if a.owned else 7):
            run(optimization+'-'+str(mode),[binary,mode,out/('fixture-'+optimization+'.tga')])
    report.update(passed=all(not row['exitCode'] for row in report['rows']),
        inputsUnchanged=all(sha(p)==digest for p,digest in inputs.items()))
    save();assert report['passed'] and report['inputsUnchanged'],out/'results.json'


if __name__=='__main__':main()
