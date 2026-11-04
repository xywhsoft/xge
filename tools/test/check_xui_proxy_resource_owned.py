"""CPU-only common native resource component qualification.

This compiles the component directly against one declared XRT host. It does
not certify the default backend integration, full native DLL, language
package or hardware operations; those require separate matching-build gates.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,time
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','runtime','out'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();root,runtime,out=[v.resolve() for v in (a.root,a.runtime,a.out)]
    cc=shutil.which('gcc');assert cc
    sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest()
    files=[Path(__file__).resolve(),Path(cc),runtime,
        *[root/name for name in ('tests/xui/xui_proxy_resource_ownership_test.c','src/xui/core/xui_proxy_resource.c','src/xui/core/xui_proxy_resource.h','include/xui.h','include/xui_config.h','include/xge_config.h','extlibs/xrt/xrt.h','extlibs/xrt/xrt_config.h','src/xui/core/xui_internal.h','src/xui/core/xui_xrt_port.h','src/xui/core/xui_unicode_core.h','modules/xlayout/xlayout.h')]]
    files.extend((root/'include').glob('*.h'))
    inputs={str(v):sha(v) for v in files};out.mkdir(parents=True,exist_ok=False)
    env=dict(os.environ)
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(complete=False,passed=False,scope=__doc__,inputs=inputs,rows=[],componentOnly=True,matchedEngineAcceptance=False,gpuTests=False,networkTests=False,
        pending=['Resident planner checked snapshot Drop failure delivery','Default backend integration','Context/widget/cache complete graph and checked cleanup'])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    for level in ('O0','O2'):
        program=out/(level+('.exe' if os.name=='nt' else ''))
        flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-cast-function-type','-DXRT_MODULE_ALL',*['-I'+str(v) for v in (root,root/'include',root/'extlibs/xrt',root/'src/xui/core')]]
        if os.name=='nt':flags+=['-DXRT_USE_SHARED']
        links=[] if os.name=='nt' else ['-pthread','-ldl','-lm','-Wl,-rpath,'+str(runtime.parent)]
        for name,command in ((level+'-build',[cc,*flags,'-'+level,root/'tests/xui/xui_proxy_resource_ownership_test.c',root/'src/xui/core/xui_proxy_resource.c',runtime,*links,'-o',program]),(level+'-run',[program])):
            start=time.monotonic();r=subprocess.run(list(map(str,command)),cwd=root,env=env,capture_output=True,timeout=120)
            log=out/(name+'.log');log.write_bytes(r.stdout+r.stderr)
            report['rows'].append(dict(name=name,command=list(map(str,command)),exitCode=r.returncode,seconds=round(time.monotonic()-start,3),logSha256=sha(log)));save()
            print(name,r.returncode,flush=True);assert not r.returncode,log
    report.update(complete=True,passed=all(sha(v)==value for v,value in inputs.items()));save();assert report['passed']
if __name__=='__main__':main()
