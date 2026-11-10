"""Qualify native caller-owned Scene transitions through the actual engine.

Headless manual frames only. This does not certify managed Scene ownership,
language callback graphs, an xlang compiler or a shipping delivery.
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
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('root','engine','runtime','host-header','out'):
        parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--test-root',type=Path)
    parser.add_argument('--owned',action='store_true',help='Qualify opaque owned Scene refs callbacks and lifecycle instead of raw records')
    args=parser.parse_args()
    root,engine,runtime,header,out=[v.resolve() for v in
        (args.root,args.engine,args.runtime,args.host_header,args.out)]
    tests=args.test_root.resolve() if args.test_root else root
    metadata=json.loads((engine/'build-report.json').read_text())
    assert metadata['linked'] and metadata['xrt_mode']=='host'
    canonical=lambda p:hashlib.sha256(p.read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    assert metadata['xrt_host_header_sha256']==canonical(header)==canonical(root/'extlibs/xrt/xrt.h')
    cc=shutil.which('gcc');assert cc
    image=engine/metadata['runtime_image']
    library=engine/metadata['import_library'] if os.name=='nt' else image
    source=tests/'tests/xge'/('xge_scene_owned_test.c' if args.owned else 'xge_scene_transition_test.c')
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    files=[Path(__file__).resolve(),Path(cc),source,header,image,library,runtime,
        engine/'build-report.json',engine/'xge_build_config.h',root/'include/xge.h',
        root/'src/xge/core/xge_core.c',root/'src/xge/core/xge_scene.c']
    if args.owned:files.append(root/'src/xge/core/xge_scene_owned.c')
    files.extend(p for p in (engine/'host-xrt').iterdir() if p.is_file())
    inputs={str(p):sha(p) for p in files}
    out.mkdir(parents=True,exist_ok=False);(out/'tmp').mkdir()
    env=dict(os.environ,TEMP=str(out/'tmp'),TMP=str(out/'tmp'),TMPDIR=str(out/'tmp'))
    if os.name=='nt':
        for p in (runtime,image):shutil.copy2(p,out/p.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+str(engine)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(passed=False,scope=__doc__,ownedScenes=args.owned,networkTests=False,graphicsDevice=False,inputs=inputs,rows=[])
    def save():
        (out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    def run(name,command):
        command=list(map(str,command));start=time.monotonic()
        result=subprocess.run(command,cwd=root,env=env,capture_output=True,timeout=120)
        log=out/(name+'.log');log.write_bytes(result.stdout+result.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=result.returncode,
            seconds=round(time.monotonic()-start,3),logSha256=sha(log)))
        save();print(name,result.returncode,flush=True);assert not result.returncode,log
    flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-DXGE_XRT_HOST=1',
        '-include',engine/'xge_build_config.h']
    flags+=['-I'+str(p) for p in (root,root/'include',engine/'host-xrt')]
    if os.name=='nt':flags+=['-DXGE_DLL','-DXUI_DLL']
    systems=[] if os.name=='nt' else ['-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine),'-pthread','-ldl','-lm']
    for optimization in ('O0','O2'):
        exe=out/(optimization+('.exe' if os.name=='nt' else ''))
        run(optimization+'-build',[cc,*flags,'-'+optimization,source,library,runtime,*systems,'-o',exe])
        for mode in range(1,19 if args.owned else 24):run(optimization+'-'+str(mode),[exe,str(mode)])
    assert all(sha(p)==value for p,value in inputs.items())
    report.update(passed=True,inputsUnchanged=True);save()


if __name__=='__main__':
    main()
