"""CPU-only Context-owned native MsgBox/Toast managers against an explicit host runtime.

GCC O0/O2 clients link the same O2 native engine. This does not certify GPU,
shipping xlang bindings, or sanitizer coverage of the engine/runtime.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,time

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','engine','runtime','host-header','out'):p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--case',choices=('managers','file_dialog','manager_getters'),default='managers')
    a=p.parse_args();root,engine,runtime,header,out=[x.resolve() for x in (a.root,a.engine,a.runtime,a.host_header,a.out)]
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    metadata=json.loads((engine/'build-report.json').read_text())
    assert metadata['linked'] and metadata['xrt_mode']=='host'
    assert metadata['xrt_host_header_sha256']==hashlib.sha256(header.read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    cc=shutil.which('gcc');assert cc
    source=root/('tests/xui/xui_owned_'+a.case+'_test.c')
    regression_names={'file_dialog':('xui_file_dialog_test.c',),'managers':('xui_msgbox_test.c','xui_toast_test.c','xui_msgtip_test.c'),'tree':('xui_tree_view_test.c','xui_tree_adapter_lifetime_test.c'),'table':('xui_table_view_test.c','xui_table_provider_lifetime_test.c'),'grid':('xui_table_grid_test.c',),'parts':tuple('xui_'+name+'_test.c' for name in ('widget','widget_type','panel','popup','menu','scroll_frame','scroll_view','tabs','split_layout','carousel','accordion','table_grid','window','combobox','numeric_input'))}
    regressions=[root/'tests/xui'/name for name in regression_names.get(a.case,())]
    library=engine/('xge.lib' if os.name=='nt' else 'libxge.so');native=engine/('xge.dll' if os.name=='nt' else 'libxge.so')
    # The Surface transaction rollback fixture exercises private slots. Link
    # the exact production native objects instead of exporting test hooks in
    # the DLL or recompiling a different implementation into the client.
    staticNative=a.case in ('surfaces','manager_getters')
    staticLegacy=a.case=='file_dialog'
    nativeObjects=[]
    if staticNative or staticLegacy:
        def native_object(name):
            folder=engine;visited=set()
            while folder not in visited:
                visited.add(folder);candidate=folder/'obj'/(name.replace('/','_').replace('\\','_')+'.o')
                if candidate.is_file():return candidate
                report=json.loads((folder/'build-report.json').read_text())
                inherited=report.get('incremental_native_overlay',{}).get('base')
                if not inherited:break
                folder=Path(inherited)
            raise FileNotFoundError(name)
        nativeObjects=[native_object(name) for name in metadata['sources']]
        inherited=Path(metadata.get('incremental_native_overlay',{}).get('base',engine))
        if 'XGE_ENABLE_HARFBUZZ' in metadata['features']:
            build=json.loads((engine/'overlay-build.json').read_text())
            shared=[Path(name) for name in build['inputs'] if name.endswith('extlibs_harfbuzz_src_harfbuzz.cc.o')]
            assert len(shared)==1;nativeObjects+=shared
        if os.name=='nt':nativeObjects+=sorted((inherited/'obj').glob('*_res.o'))
        assert all(path.is_file() for path in nativeObjects)
    pins={Path(__file__).resolve(),Path(cc),source,header,library,native,runtime,engine/'build-report.json',engine/'xge_build_config.h',root/'include/xge.h',root/'include/xui.h',root/'include/xge_host_xrt.h',root/'extlibs/xrt/xrt.h'}
    pins.update(regressions);pins.update(nativeObjects)
    if a.case=='receipt':pins.add(root/'tests/xui/xui_owned_fonts_test.c')
    if regressions or a.case in ('table','parts','grid','fonts','receipt','surfaces','managers','file_dialog','manager_getters'):pins.update(root/'tests/xui'/name for name in ('xui_test_proxy.h','xui_test_proxy.c'))
    pins.update(root/x for x in metadata['sources']);pins.add(root/'src/xui/core/xui_internal.h');pins.add(root/'src/xui/core/xui_xrt_port.h');pins.add(root/'include/xui_config.h');pins.update(x for x in (engine/'host-xrt').iterdir() if x.is_file());inputs={str(x):sha(x) for x in pins}
    out.mkdir(parents=True,exist_ok=False);(out/'tmp').mkdir()
    env=dict(os.environ,TEMP=str(out/'tmp'),TMP=str(out/'tmp'),TMPDIR=str(out/'tmp'))
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name);shutil.copy2(native,out/native.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+str(engine)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(complete=False,passed=False,scope=__doc__,networkTests=False,staticProductionNativeObjects=staticNative,staticLegacyProductionObjects=staticLegacy,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    def run(name,command):
        start=time.monotonic()
        try:
            q=subprocess.run(list(map(str,command)),cwd=root,env=env,capture_output=True,timeout=90)
            code,output=q.returncode,q.stdout+q.stderr
        except subprocess.TimeoutExpired as failure:
            code,output=124,(failure.stdout or b'')+(failure.stderr or b'')
        log=out/(name+'.log');log.write_bytes(output);report['rows'].append(dict(name=name,command=list(map(str,command)),exitCode=code,seconds=round(time.monotonic()-start,3),logSha256=sha(log)));save();print(name,code,flush=True);assert not code,log
    flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-Wno-unused-parameter','-DXGE_XRT_HOST=1','-include',str(engine/'xge_build_config.h')]
    flags+=['-I'+str(x) for x in (root,root/'include',engine/'host-xrt')]
    if os.name=='nt' and not staticNative:flags+=['-DXGE_DLL','-DXUI_DLL']
    systems=['-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine),'-pthread','-ldl','-lm'] if os.name!='nt' else []
    if staticNative:
        systems+=['-lm','-lws2_32','-liphlpapi','-lgdi32','-luser32','-lshell32','-lole32','-loleaut32','-luuid','-limm32','-lwinmm','-lavrt'] if os.name=='nt' else ['-lX11','-lXi','-lXcursor','-lGL']
    nativeLink=nativeObjects if staticNative else [library]
    for opt in ('O0','O2'):
        exe=out/('owned-'+opt+('.exe' if os.name=='nt' else ''))
        test_extra=['-I'+str(root/'tests/xui'),root/'tests/xui/xui_test_proxy.c'] if a.case in ('table','parts','grid','fonts','receipt','surfaces','managers','file_dialog','manager_getters') else []
        if a.case in ('parts','fonts','receipt','surfaces','managers','file_dialog','manager_getters'):test_extra+=['-Wno-unused-function']
        run(opt+'-build',[cc,*flags,'-'+opt,source,*test_extra,*nativeLink,runtime,*systems,'-o',exe]);run(opt+'-run',[exe])
    if regressions:
        legacyLink=[library];legacyFlags=list(flags);legacySystems=list(systems)
        if staticLegacy:
            # Production DLL contains no test exports. The pre-existing pure
            # path getter seams compile only into this separate test object;
            # the public business body is the exact same production source.
            testObject=out/'file-dialog-test-native.o'
            compileFlags=list(metadata['flags'])
            run('file-dialog-test-native-build',[cc,*compileFlags,'-DXUI_FILE_DIALOG_TEST=1','-c',root/'src/xui/controls/xui_file_dialog.c','-o',testObject])
            legacyLink=[testObject if value.name=='src_xui_controls_xui_file_dialog.c.o' else value for value in nativeObjects]
            legacyFlags=[value for value in flags if value not in ('-DXGE_DLL','-DXUI_DLL')]
            legacySystems+=['-lm','-lws2_32','-liphlpapi','-lgdi32','-luser32','-lshell32','-lole32','-loleaut32','-luuid','-limm32','-lwinmm','-lavrt'] if os.name=='nt' else ['-lX11','-lXi','-lXcursor','-lGL']
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
                run(name+'-build',[cc,*legacyFlags,'-I'+str(root/'tests/xui'),'-I'+str(root/'tests'),'-Wno-unused-function','-'+opt,wrapper,root/'tests/xui/xui_test_proxy.c',*legacyLink,runtime,*legacySystems,'-o',exe]);run(name+'-run',[exe])
    report['inputsUnchanged']=all(sha(k)==v for k,v in inputs.items());assert report['inputsUnchanged'];report.update(complete=True,passed=True);save()
if __name__=='__main__':main()
