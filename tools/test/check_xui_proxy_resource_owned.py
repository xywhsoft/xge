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
    p.add_argument('--case',choices=('resource','draw','pass'),default='resource')
    a=p.parse_args();root,runtime,out=[v.resolve() for v in (a.root,a.runtime,a.out)]
    test=root/('tests/xge/xge_pass_order_test.c' if a.case=='pass' else 'tests/xui/xui_owned_draw_test.c' if a.case=='draw' else 'tests/xui/xui_proxy_resource_ownership_test.c')
    cc=shutil.which('gcc');assert cc
    sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest()
    files=[Path(__file__).resolve(),Path(cc),runtime,test,
        *[root/name for name in ('tests/xui/xui_proxy_resource_ownership_test.c','src/xui/core/xui_proxy_resource.c','src/xui/core/xui_proxy_resource.h','include/xui.h','include/xui_config.h','include/xge_config.h','extlibs/xrt/xrt.h','extlibs/xrt/xrt_config.h','src/xui/core/xui_internal.h','src/xui/core/xui_xrt_port.h','src/xui/core/xui_unicode_core.h','modules/xlayout/xlayout.h')]]
    if a.case=='pass':files.extend(root/name for name in ('src/xge/render/xge_pass.inl','src/xge/render/xge_pass_state.h','src/xui/core/xui_proxy_xge.c'))
    files.extend((root/'include').glob('*.h'))
    inputs={str(v):sha(v) for v in files};out.mkdir(parents=True,exist_ok=False)
    if a.case=='pass':
        # Compile the exact production Draw validation and representative entry
        # bodies against CPU renderer doubles, rather than hand-copying them.
        source=(root/'src/xui/core/xui_proxy_xge.c').read_text(encoding='utf-8')
        def definition(start):
            begin=source.index(start);at=source.index('{',begin);depth=1;end=at+1
            while depth:
                depth += (source[end]=='{')-(source[end]=='}');end+=1
            return source[begin:end]
        parts=['#define XUI_PROXY_XGE_DRAW_MAGIC 0x58554944u',definition('struct xui_draw_context_t {')+';']
        for name in ('DrawValid','DrawMarkDirty','DrawClearRect','DrawSurface','DrawPoint','TargetEnd'):
            spelling=('static void ' if name=='DrawMarkDirty' else 'static int ')+'__xuiProxyXge'+name+'('
            parts.append(definition(spelling))
        generated=out/'pass_draw_methods.inc';generated.write_text('\n\n'.join(parts)+'\n',encoding='utf-8')
        reportGenerated=dict(path=str(generated),sha256=sha(generated),productionSource=str(root/'src/xui/core/xui_proxy_xge.c'))
    env=dict(os.environ)
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(complete=False,passed=False,scope=__doc__,case=a.case,inputs=inputs,rows=[],componentOnly=True,matchedEngineAcceptance=False,gpuTests=False,networkTests=False,
        pending=['Resident planner checked snapshot Drop failure delivery','Default backend integration','Context/widget/cache complete graph and checked cleanup'])
    if a.case=='pass':report['generatedProductionBodies']=reportGenerated
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    for level in ('O0','O2'):
        program=out/(level+('.exe' if os.name=='nt' else ''))
        flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-cast-function-type','-DXRT_MODULE_ALL',*['-I'+str(v) for v in (root,root/'include',root/'extlibs/xrt',root/'src/xui/core')]]
        if a.case=='pass':flags+=['-I'+str(out),'-DXGE_PASS_DRAW_METHODS=\"pass_draw_methods.inc\"']
        if os.name=='nt':flags+=['-DXRT_USE_SHARED']
        links=[] if os.name=='nt' else ['-pthread','-ldl','-lm','-Wl,-rpath,'+str(runtime.parent)]
        for name,command in ((level+'-build',[cc,*flags,'-'+level,test,root/'src/xui/core/xui_proxy_resource.c',runtime,*links,'-o',program]),(level+'-run',[program])):
            start=time.monotonic();r=subprocess.run(list(map(str,command)),cwd=root,env=env,capture_output=True,timeout=120)
            log=out/(name+'.log');log.write_bytes(r.stdout+r.stderr)
            report['rows'].append(dict(name=name,command=list(map(str,command)),exitCode=r.returncode,seconds=round(time.monotonic()-start,3),logSha256=sha(log)));save()
            print(name,r.returncode,flush=True);assert not r.returncode,log
    report.update(complete=True,passed=all(sha(v)==value for v,value in inputs.items()));save();assert report['passed']
if __name__=='__main__':main()
