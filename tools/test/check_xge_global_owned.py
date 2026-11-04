"""CPU-only XGE global Owned callback/lifecycle test with one explicit XRT host.

No window, graphics device, network or desktop input is initialized. This gate
does not certify language declarations, GPU behavior or shipping packages.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,time

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('root','engine','runtime','host-header','out'):parser.add_argument('--'+name,type=Path,required=True)
    a=parser.parse_args();root,engine,runtime,header,out=[p.resolve() for p in (a.root,a.engine,a.runtime,a.host_header,a.out)]
    metadata=json.loads((engine/'build-report.json').read_text(encoding='utf-8'))
    assert metadata['linked'] and metadata['xrt_mode']=='host'
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    assert metadata['xrt_host_header_sha256']==hashlib.sha256(header.read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    cc=shutil.which('gcc');assert cc
    binary=engine/('xge.dll' if os.name=='nt' else 'libxge.so')
    library=engine/('xge.lib' if os.name=='nt' else 'libxge.so')
    source=root/'tests/xge/xge_owned_global_test.c'
    pins={Path(__file__).resolve(),Path(cc),source,runtime,header,binary,library,engine/'build-report.json',engine/'xge_build_config.h',
          root/'include/xge.h',root/'include/xge_config.h',root/'include/xge_host_xrt.h',root/'extlibs/xrt/xrt.h',root/'extlibs/xrt/xrt_config.h',
          root/'src/xge/core/xge_impl.c',root/'src/xge/core/xge_core.c',root/'src/xge/core/xge_callback_owned.h',root/'src/xge/input/xge_drag_drop.c'}
    pins.update(p for p in (engine/'host-xrt').iterdir() if p.is_file())
    inputs={str(p):sha(p) for p in pins};out.mkdir(parents=True,exist_ok=False);(out/'tmp').mkdir()
    env=dict(os.environ,TEMP=str(out/'tmp'),TMP=str(out/'tmp'),TMPDIR=str(out/'tmp'))
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name);shutil.copy2(binary,out/binary.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+str(engine)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(complete=False,passed=False,scope=__doc__,inputs=inputs,rows=[],networkTests=False)
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    def run(name,command):
        start=time.monotonic();r=subprocess.run(list(map(str,command)),cwd=root,env=env,capture_output=True,timeout=240)
        log=out/(name+'.log');log.write_bytes(r.stdout+r.stderr)
        report['rows'].append(dict(name=name,command=list(map(str,command)),exitCode=r.returncode,seconds=round(time.monotonic()-start,3),logSha256=sha(log)))
        save();print(name,r.returncode,flush=True);assert not r.returncode,log
    flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-DXGE_XRT_HOST=1','-include',engine/'xge_build_config.h']
    flags+=['-I'+str(p) for p in (root,root/'include',engine/'host-xrt')]
    if os.name=='nt':flags+=['-DXGE_DLL']
    systems=[] if os.name=='nt' else ['-pthread','-ldl','-lm','-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine)]
    for opt in ('O0','O2'):
        exe=out/(opt+('.exe' if os.name=='nt' else ''))
        run(opt+'-build',[cc,*flags,'-'+opt,source,library,runtime,*systems,'-o',exe]);run(opt+'-run',[exe])
    report['inputsUnchanged']=all(sha(p)==value for p,value in inputs.items());assert report['inputsUnchanged']
    report.update(complete=True,passed=True);save()
if __name__=='__main__':main()
