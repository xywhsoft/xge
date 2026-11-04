"""Headless frame publisher/first-outcome acceptance using the actual hosted engine.

No engine initialization, Sokol loop, window, GPU or audio device is executed.
Blocking Run outcome capture uses the actual shared accumulator helpers;
manual/mobile persistence follows the native registration path and source
contract, while the independent dispatch is exercised as real native code.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,time
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','engine','runtime','host-header','out'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();root,engine,runtime,header,out=[v.resolve() for v in (a.root,a.engine,a.runtime,a.host_header,a.out)]
    metadata=json.loads((engine/'build-report.json').read_text());assert metadata['linked'] and metadata['xrt_mode']=='host'
    sha=lambda path:hashlib.sha256(Path(path).read_bytes()).hexdigest()
    assert metadata['xrt_host_header_sha256']==hashlib.sha256(header.read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    cc=shutil.which('gcc');assert cc
    binary=engine/('xge.dll' if os.name=='nt' else 'libxge.so');library=engine/('xge.lib' if os.name=='nt' else 'libxge.so')
    source=root/'tests/xge/xge_owned_frame_test.c'
    pins=[Path(__file__).resolve(),Path(cc),source,runtime,header,binary,library,engine/'build-report.json',engine/'xge_build_config.h',
        root/'include/xge.h',root/'include/xge_host_xrt.h',root/'extlibs/xrt/xrt.h',root/'extlibs/xrt/xrt_config.h',
        root/'src/xge/core/xge_core.c',root/'src/xge/core/xge_impl.c',root/'src/xge/core/xge_callback_owned.h',
        *[path for path in (engine/'host-xrt').iterdir() if path.is_file()]]
    inputs={str(path):sha(path) for path in pins};out.mkdir(parents=True,exist_ok=False)
    env=dict(os.environ)
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name);shutil.copy2(binary,out/binary.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+str(engine)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(scope=__doc__,complete=False,passed=False,inputs=inputs,rows=[],gpuTests=False,networkTests=False)
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    for level in ('O0','O2'):
        program=out/(level+('.exe' if os.name=='nt' else ''))
        flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-cast-function-type','-DXGE_XRT_HOST=1',
            '-include',engine/'xge_build_config.h',*['-I'+str(v) for v in (root,root/'include',engine/'host-xrt')]]
        if os.name=='nt':flags+=['-DXGE_DLL']
        links=[] if os.name=='nt' else ['-pthread','-ldl','-lm','-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine)]
        for name,command in ((level+'-build',[cc,*flags,'-'+level,source,library,runtime,*links,'-o',program]),(level+'-run',[program])):
            start=time.monotonic();r=subprocess.run(list(map(str,command)),cwd=root,env=env,capture_output=True,timeout=240)
            log=out/(name+'.log');log.write_bytes(r.stdout+r.stderr)
            report['rows'].append(dict(name=name,command=list(map(str,command)),exitCode=r.returncode,seconds=round(time.monotonic()-start,3),logSha256=sha(log)));save()
            print(name,r.returncode,flush=True);assert not r.returncode,log
    report.update(complete=True,passed=all(sha(path)==value for path,value in inputs.items()));save();assert report['passed']
if __name__=='__main__':main()
