"""CPU-only checked borrowed text read views and existing editor regressions.

GCC O0/O2 clients use the explicit O2 host engine and one supplied XRT runtime.
Focused borrowed read paths; rendering, other mutations and full editor
callback ownership remain separate qualification.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,time

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('root','engine','runtime','out'):parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();root,engine,runtime,out=[p.resolve()for p in(args.root,args.engine,args.runtime,args.out)]
    win=os.name=='nt';cc=Path(shutil.which('gcc')).resolve();sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    metadata=json.loads((engine/'build-report.json').read_text(encoding='utf-8'))
    assert metadata['linked']and metadata['xrt_mode']=='host'
    native=engine/('xge3_host.dll'if win else'libxge3_host.so');library=engine/'xge3_host.lib'if win else native
    sources=[root/'tests/xui'/n for n in('xui_text_read_view_test.c','xui_edit_contract_test.c','xui_code_edit_test.c','xui_text_edit_test.c')]
    files=[Path(__file__),cc,native,library,runtime,engine/'build-report.json',engine/'xge_build_config.h',*sources,root/'src/xui/code/xui_code_document.c',root/'src/xui/core/xui_edit.c',root/'extlibs/xrt/xrt.h',root/'extlibs/xrt/xrt_config.h',root/'tests/xui/xui_test_proxy.c',root/'tests/xui/xui_test_proxy.h']
    files.extend(p for p in(engine/'host-xrt').rglob('*')if p.is_file())
    files.extend(p for p in(root/'include').rglob('*.h')if p.is_file())
    inputs={str(p):sha(p)for p in files};out.mkdir(parents=True,exist_ok=False)
    env=dict(os.environ)
    if win:
        shutil.copy2(runtime,out/runtime.name);shutil.copy2(native,out/native.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+os.pathsep+str(engine)+os.pathsep+env.get('LD_LIBRARY_PATH','')
    report=dict(complete=False,passed=False,scope=__doc__,networkTests=False,inputs=inputs,rows=[])
    def save():
        p=out/'results.json.tmp';p.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');os.replace(p,out/'results.json')
    def run(name,command):
        command=list(map(str,command));start=time.monotonic();z=subprocess.run(command,cwd=root,env=env,capture_output=True,timeout=180)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=z.returncode,seconds=time.monotonic()-start,logSha256=sha(log)))
        save();print(name,z.returncode,flush=True);assert not z.returncode,log
    flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-parameter','-Wno-cast-function-type','-DXGE_XRT_HOST=1','-include',engine/'xge_build_config.h']
    flags+=['-I'+str(p)for p in(root,root/'include',root/'tests/xui',engine/'host-xrt')]
    if win:flags+=['-DXGE_DLL=1','-DXUI_DLL=1']
    systems=[]if win else['-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine),'-pthread','-ldl','-lm']
    try:
        for case in sources:
            wrapper=out/(case.stem+'-host.c')
            wrapper.write_text('#define XRT_MODULE_ALL\n#include "extlibs/xrt/xrt.h"\n#include "xge_host_xrt.h"\n#include <xge_host_xrt_contract.h>\n#include <assert.h>\n'
                'extern xge_host_xrt_proc xl6HostXrtResolveV1(const char*);\n'
                'static xge_host_xrt_proc resolve(void*u,const char*n){(void)u;return xl6HostXrtResolveV1(n);}\n'
                '#define main icon_test_body\n#include "'+case.as_posix()+'"\n#undef main\n'
                'int main(void){const xge_host_xrt_contract_t c=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&c,resolve,NULL,NULL)==XGE_HOST_XRT_OK);return icon_test_body();}\n',encoding='utf-8')
            for opt in('O0','O2'):
                name=case.stem+'-'+opt;exe=out/(name+('.exe'if win else''));extra=[root/'tests/xui/xui_test_proxy.c']
                run(name+'-build',[cc,*flags,'-'+opt,wrapper,*extra,library,runtime,*systems,'-o',exe]);run(name+'-run',[exe])
        report.update(complete=True,inputsUnchanged=all(sha(Path(p))==h for p,h in inputs.items()))
        report['passed']=report['inputsUnchanged'];assert report['passed']
    except Exception as error:report.update(complete=True,passed=False,error=str(error));raise
    finally:save()
if __name__=='__main__':main()
