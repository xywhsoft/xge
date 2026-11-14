"""Choice constructor failure cleanup and isolated external-image cache refresh.
Matched O2 hosted/static SDKs; O0/O2 independent C, no language/network or
shipping-delivery certification. Unchanged Checkbox/Toggle/Radio suites are
included; Button's older proxy-geometry fixture is not silently substituted.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,time


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('root','engine','static','runtime','out'):
        p.add_argument('--'+n,type=Path,required=True)
    a=p.parse_args();root,engine,static,runtime,out=[v.resolve()for v in
        (a.root,a.engine,a.static,a.runtime,a.out)]
    win=os.name=='nt';cc=Path(shutil.which('gcc')).resolve()
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    h=json.loads((engine/'build-report.json').read_text(encoding='utf-8'))
    s=json.loads((static/'build-report.json').read_text(encoding='utf-8'))
    assert h['linked']and h['xrt_mode']=='host'and s['passed']and s['sourceUnchanged']
    image=engine/h['runtime_image'];lib=engine/h['import_library']if win else image
    archive=static/s['archive'];source=root/'tests/xui/xui_choice_init_failure_test.c'
    probe=root/'tests/xui/xui_choice_surface_generation_test.c'
    legacy=[root/('tests/xui/xui_'+n+'_test.c')for n in('checkbox','toggle','radio')]
    files=[Path(__file__),cc,source,probe,*legacy,image,lib,archive,runtime,
        engine/'build-report.json',static/'build-report.json',engine/'xge_build_config.h',
        static/'xge_build_config.h',static/'xge_static_xrt_contract.h',
        *sorted((engine/'host-xrt').glob('*')),*sorted((root/'include').glob('*.h'))]
    files.extend(root/n for n in('src/xui/controls/xui_button.c','src/xui/controls/xui_checkbox.c',
        'src/xui/controls/xui_toggle.c','src/xui/controls/xui_radio.c','src/xui/core/xui_widget.c',
        'src/xui/core/xui_internal.h','tests/xui/xui_test_proxy.c','tests/xui/xui_test_proxy.h',
        'extlibs/xrt/xrt.h','extlibs/xrt/xrt_config.h'))
    inputs={str(v):sha(v)for v in files};out.mkdir(parents=True,exist_ok=False)
    for v in(source,probe):shutil.copy2(v,out/v.name)
    r=dict(complete=False,passed=False,scope=__doc__,shippingDelivery=False,
        languageBindingsQualified=False,networkTests=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_bytes((json.dumps(r,indent=2)+'\n').encode())
    env=dict(os.environ);key='PATH'if win else'LD_LIBRARY_PATH'
    env[key]=os.pathsep.join((str(engine),str(runtime.parent),env.get(key,'')))
    def no_core():
        import resource
        resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    def run(name,cmd):
        cmd=list(map(str,cmd));start=time.monotonic()
        z=subprocess.run(cmd,cwd=root,env=env,capture_output=True,timeout=180,
            preexec_fn=None if win else no_core)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        r['rows'].append(dict(name=name,command=cmd,exitCode=z.returncode,
            seconds=time.monotonic()-start,logSha256=sha(log)))
        save();print(name,z.returncode,flush=True);assert not z.returncode,log
        return z.stdout+z.stderr
    try:
        prefix=source.read_text(encoding='utf-8').split('static size_t backendReferences=1;')[0]
        wrappers=[]
        for case in legacy:
            wrapper=out/(case.stem+'-hosted.c')
            wrapper.write_text(prefix+'\n#define main original_test_main\n#include "'+case.as_posix()+'"\n#undef main\n'+'''
int main(void){
#if defined(XGE_TEST_STATIC)
 const xge_host_xrt_contract_t c=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;assert(!strcmp(c.headerSha256,xgeStaticXrtContractV1.headerSha256));
#else
 const xge_host_xrt_contract_t c=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&c,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
 return original_test_main();
}
''',encoding='utf-8')
            wrappers.append(wrapper)
        for kind,folder,library in(('host',engine,lib),('static',static,archive)):
            flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type',
                '-Wno-unused-function','-DXGE_XRT_HOST=1','-include',folder/'xge_build_config.h',
                *['-I'+str(v)for v in(root,root/'include',folder,engine/'host-xrt')]]
            flags+=['-DXGE_TEST_STATIC=1']if kind=='static'else['-DXGE_DLL=1','-DXUI_DLL=1']if win else[]
            systems=['-l'+v for v in s['system_libraries']]if kind=='static'else[]
            if not win:systems+=['-Wl,-rpath,'+str(engine),'-Wl,-rpath,'+str(runtime.parent),'-ldl','-lm','-pthread']
            for opt in('O0','O2'):
                for label,case in [('initializers',source),*[(v.stem,v)for v in wrappers],('isolated-cache',probe)]:
                    name=kind+'-'+opt+'-'+label;binary=out/(name+('.exe'if win else''))
                    run(name+'-build',[cc,*flags,'-'+opt,case,library,runtime,*systems,'-o',binary])
                    if case==probe:
                        for i in range(4):
                            data=run(name+'-'+str(i),[binary,str(i)])
                            assert b'clean-cache generation refresh=1'in data
                    else:
                        data=run(name,[binary])
                        if case==source:assert b'512 allocation prefixes, NULL outputs and exact heap cleanup passed'in data
        assert len(r['rows'])==52
        r.update(complete=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()))
        r['passed']=r['inputsUnchanged'];assert r['passed']
    except Exception as e:r.update(complete=True,passed=False,error=str(e));raise
    finally:save()


if __name__=='__main__':main()
