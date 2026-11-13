"""CPU DecodeBudget RC/charges, 3D CPU regressions and MODEL=0 isolation.

The full O2 host uses one supplied XRT. O0/O2 clients run the budget, CPU
hierarchy and spatial cases. A second program compiles just the quota TU
against that XRT with MODEL/ANIMATION/ASYNC/CACHE disabled, and verifies it
imports no XGE engine/model DLL. No GPU, model-file/network or parser test.
"""
from pathlib import Path
import argparse,hashlib,json,os,re,shutil,subprocess,time


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in('root','engine','runtime','out'):parser.add_argument('--'+name,type=Path,required=True)
    a=parser.parse_args();root,engine,runtime,out=[p.resolve()for p in(a.root,a.engine,a.runtime,a.out)]
    win=os.name=='nt';tools={n:Path(shutil.which(n)).resolve()for n in('gcc','g++','nm','objdump')}
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    metadata=json.loads((engine/'build-report.json').read_text());assert metadata['linked']and metadata['xrt_mode']=='host'
    native=engine/('xge3_host.dll'if win else'libxge3_host.so');library=engine/'xge3_host.lib'if win else native
    sources=[root/'tests/xge'/n for n in('test_3d_decode_budget.c','test_3d_cpu.c','test_3d_spatial.c')]
    quota=root/'src/xge/3d/xge3d_decode_budget.c'
    files=[Path(__file__),*tools.values(),native,library,runtime,engine/'build-report.json',
        engine/'xge_build_config.h',*sources,quota,root/'config/modules.json',
        root/'src/xge/3d/xge3d_instance.inc',root/'extlibs/xrt/xrt.h',root/'extlibs/xrt/xrt_config.h']
    files.extend(p for p in(root/'include').glob('*.h'))
    files.extend(p for p in(engine/'host-xrt').rglob('*')if p.is_file())
    inputs={str(p):sha(p)for p in files};out.mkdir(parents=True,exist_ok=False)
    env=os.environ.copy();key='PATH'if win else'LD_LIBRARY_PATH'
    env[key]=str(runtime.parent)+os.pathsep+str(engine)+os.pathsep+env.get(key,'')
    report=dict(complete=False,passed=False,scope=__doc__,networkTests=False,inputs=inputs,rows=[],minimalImports=[])
    def save():
        p=out/'results.json.tmp';p.write_text(json.dumps(report,indent=2)+'\n');os.replace(p,out/'results.json')
    def run(name,command):
        command=list(map(str,command));start=time.monotonic()
        z=subprocess.run(command,cwd=root,env=env,capture_output=True,timeout=180)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=z.returncode,
            seconds=time.monotonic()-start,logSha256=sha(log)))
        save();print(name,z.returncode,flush=True);assert not z.returncode,log
        return z.stdout+z.stderr
    flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-missing-field-initializers',
        '-DXGE_XRT_HOST=1','-include',engine/'xge_build_config.h']
    flags+=['-I'+str(p)for p in(root,root/'include',engine/'host-xrt')]
    if win:flags+=['-DXGE_DLL=1','-DXUI_DLL=1']
    systems=[]if win else['-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine),'-pthread','-ldl','-lm']
    minimal=['-DXGE_ENABLE_3D=1','-DXGE3D_ENABLE_MODEL=0','-DXGE3D_ENABLE_ANIMATION=0',
        '-DXGE3D_ENABLE_ASYNC=0','-DXGE3D_ENABLE_CACHE=0','-I'+str(root),'-I'+str(root/'include')]
    try:
        for case in sources:
            wrapper=out/(case.stem+'-host.c')
            wrapper.write_text('#define XRT_MODULE_ALL\n#include "extlibs/xrt/xrt.h"\n'
                '#include "xge_host_xrt.h"\n#include <xge_host_xrt_contract.h>\n#include <assert.h>\n'
                'extern xge_host_xrt_proc xl6HostXrtResolveV1(const char*);\n'
                'static xge_host_xrt_proc resolve(void*u,const char*n){(void)u;return xl6HostXrtResolveV1(n);}\n'
                '#define main budget_test_body\n#include "'+case.as_posix()+'"\n#undef main\n'
                'int main(void){const xge_host_xrt_contract_t c=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;'
                'assert(xgeHostXrtBind(&c,resolve,NULL,NULL)==XGE_HOST_XRT_OK);return budget_test_body();}\n')
            for level in('O0','O2'):
                name=case.stem+'-'+level;exe=out/(name+('.exe'if win else''))
                run(name+'-build',[tools['gcc'],*flags,'-'+level,wrapper,library,runtime,*systems,'-o',exe])
                run(name+'-run',[exe])
        for level in('O0','O2'):
            name='model-off-'+level;obj=out/(name+'.o');exe=out/(name+('.exe'if win else''))
            run(name+'-object',[tools['gcc'],'-std=c11','-'+level,'-Wall','-Wextra','-Werror',*minimal,'-c',quota,'-o',obj])
            symbols=run(name+'-symbols',[tools['nm'],'-u',obj])
            assert b'xge3dModel'not in symbols and b'cgltf'not in symbols and not re.search(rb'\b(?:gl|egl)[A-Z]',symbols)
            run(name+'-build',[tools['gcc'],'-std=c11','-'+level,'-Wall','-Wextra','-Werror',*minimal,sources[0],obj,runtime,*systems,'-o',exe])
            run(name+'-run',[exe])
            table=run(name+'-imports',[tools['objdump'],'-p',exe])
            imports=re.findall(rb'DLL Name:\s*(\S+)',table)if win else re.findall(rb'\bNEEDED\s+(\S+)',table)
            assert not any(b'xge' in n.lower()for n in imports)
            report['minimalImports'].append(dict(level=level,imports=[n.decode()for n in imports]))
        header=out/'model-off-header.h'
        header.write_text('#include "xge.h"\n#if XGE3D_ENABLE_MODEL\n#error expected model disabled\n#endif\n'
            'typedef int (*create_type)(uint64_t,xge3d_decode_budget**);\n'
            'typedef int (*share_type)(xge3d_decode_budget*,xge3d_decode_budget**);\n'
            'create_type budget_create=&xge3dDecodeBudgetCreate;\nshare_type budget_share=&xge3dDecodeBudgetShareOwned;\n')
        for standard,tool,language in(('c11','gcc','c'),('c++11','g++','c++'),('c++17','g++','c++')):
            run('model-off-header-'+standard,[tools[tool],'-std='+standard,'-Wall','-Wextra','-Werror',*minimal,'-x',language,'-fsyntax-only',header])
        report.update(complete=True,inputsUnchanged=all(sha(p)==h for p,h in inputs.items()))
        report['passed']=report['inputsUnchanged'];assert report['passed']
    except Exception as error:report.update(complete=True,passed=False,error=str(error));raise
    finally:save()


if __name__=='__main__':main()
