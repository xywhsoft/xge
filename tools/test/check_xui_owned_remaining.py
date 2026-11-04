"""CPU-only remaining XUI Owned publishers against an explicit host runtime.

GCC O0/O2 clients link the same O2 native engine. This does not certify GPU,
shipping xlang bindings, or sanitizer coverage of the engine/runtime.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,time

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','engine','runtime','host-header','out'):p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--case',choices=('remaining','groups','tree','cell','table','context','port'),default='remaining')
    a=p.parse_args();root,engine,runtime,header,out=[x.resolve() for x in (a.root,a.engine,a.runtime,a.host_header,a.out)]
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    metadata=json.loads((engine/'build-report.json').read_text())
    assert metadata['linked'] and metadata['xrt_mode']=='host'
    assert metadata['xrt_host_header_sha256']==hashlib.sha256(header.read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    cc=shutil.which('gcc');assert cc
    source=root/('tests/xui/xui_owned_'+a.case+'_test.c')
    regression_names={'tree':('xui_tree_view_test.c','xui_tree_adapter_lifetime_test.c'),'table':('xui_table_view_test.c','xui_table_provider_lifetime_test.c')}
    regressions=[root/'tests/xui'/name for name in regression_names.get(a.case,())]
    library=engine/('xge.lib' if os.name=='nt' else 'libxge.so');native=engine/('xge.dll' if os.name=='nt' else 'libxge.so')
    pins={Path(__file__).resolve(),Path(cc),source,header,library,native,runtime,engine/'build-report.json',engine/'xge_build_config.h',root/'include/xge.h',root/'include/xui.h',root/'include/xge_host_xrt.h',root/'extlibs/xrt/xrt.h'}
    pins.update(regressions);
    if regressions or a.case=='table':pins.update(root/'tests/xui'/name for name in ('xui_test_proxy.h','xui_test_proxy.c'))
    pins.update(root/x for x in metadata['sources']);pins.add(root/'src/xui/core/xui_internal.h');pins.add(root/'src/xui/core/xui_xrt_port.h');pins.add(root/'include/xui_config.h');pins.update(x for x in (engine/'host-xrt').iterdir() if x.is_file());inputs={str(x):sha(x) for x in pins}
    out.mkdir(parents=True,exist_ok=False);(out/'tmp').mkdir()
    env=dict(os.environ,TEMP=str(out/'tmp'),TMP=str(out/'tmp'),TMPDIR=str(out/'tmp'))
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name);shutil.copy2(native,out/native.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+str(engine)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(complete=False,passed=False,scope=__doc__,networkTests=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    def run(name,command):
        start=time.monotonic();q=subprocess.run(list(map(str,command)),cwd=root,env=env,capture_output=True,timeout=240)
        log=out/(name+'.log');log.write_bytes(q.stdout+q.stderr);report['rows'].append(dict(name=name,command=list(map(str,command)),exitCode=q.returncode,seconds=round(time.monotonic()-start,3),logSha256=sha(log)));save();print(name,q.returncode,flush=True);assert not q.returncode,log
    flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-Wno-unused-parameter','-DXGE_XRT_HOST=1','-include',str(engine/'xge_build_config.h')]
    flags+=['-I'+str(x) for x in (root,root/'include',engine/'host-xrt')]
    if os.name=='nt':flags+=['-DXGE_DLL','-DXUI_DLL']
    systems=['-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine),'-pthread','-ldl','-lm'] if os.name!='nt' else []
    for opt in ('O0','O2'):
        exe=out/('owned-'+opt+('.exe' if os.name=='nt' else ''))
        test_extra=['-I'+str(root/'tests/xui'),root/'tests/xui/xui_test_proxy.c'] if a.case=='table' else []
        run(opt+'-build',[cc,*flags,'-'+opt,source,*test_extra,library,runtime,*systems,'-o',exe]);run(opt+'-run',[exe])
    if regressions:
        prefix=source.read_text(encoding='utf8').split('typedef struct TreeState' if a.case=='tree' else '#include "xui_test_proxy.h"')[0]
        for regression in regressions:
            wrapper=out/(regression.stem+'-host.c')
            original=regression.read_text(encoding='utf8')
            old_main='int main(void)' if 'int main(void)' in original else 'int main(int argc, char** argv)'
            new_main='int xui_legacy_main(void)' if old_main=='int main(void)' else 'int xui_legacy_main(int argc, char** argv)'
            invocation='xui_legacy_main()' if old_main=='int main(void)' else 'xui_legacy_main(0,NULL)'
            assert original.count(old_main)==1
            wrapper.write_text(prefix+'\n'+original.replace(old_main,new_main,1)+'\nint main(void){const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);return '+invocation+';}\n',encoding='utf8')
            for opt in ('O0','O2'):
                name=regression.stem+'-'+opt
                exe=out/(name+('.exe' if os.name=='nt' else ''))
                run(name+'-build',[cc,*flags,'-I'+str(root/'tests/xui'),'-I'+str(root/'tests'),'-Wno-unused-function','-'+opt,wrapper,root/'tests/xui/xui_test_proxy.c',library,runtime,*systems,'-o',exe]);run(name+'-run',[exe])
    report['inputsUnchanged']=all(sha(k)==v for k,v in inputs.items());assert report['inputsUnchanged'];report.update(complete=True,passed=True);save()
if __name__=='__main__':main()
