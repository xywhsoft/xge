"""Actual CodeDocument search/replace counted text, normalized advancement and cleanup outcome.

Exact hosted/static engine, one full XRT, O0/O2 clients, no UI/network tests.
Baseline clients require exact counted-text/CRLF failures 83/84; cleanup
priority, callback close/failure and allocation prefixes stay positive controls.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('root','test-root','engine','static','runtime','out'):p.add_argument('--'+n,type=Path,required=True)
    p.add_argument('--baseline',action='store_true',help='Expect exact known outcome failures 83/84; no product source overlay')
    p.add_argument('--unicode-baseline',action='store_true',help='Only test bounded r1 partial-grapheme failure 86; never use the original unbounded algorithm')
    a=p.parse_args();root,tests,engine,static,runtime,out=[v.resolve()for v in(a.root,a.test_root,a.engine,a.static,a.runtime,a.out)]
    win=os.name=='nt';cc=Path(shutil.which('gcc')).resolve()
    assert not(a.baseline and a.unicode_baseline)
    h=json.loads((engine/'build-report.json').read_text(encoding='utf-8'));s=json.loads((static/'build-report.json').read_text(encoding='utf-8'))
    assert h['linked']and h['xrt_mode']=='host'and s['passed']and s['sourceUnchanged']and s['xrt_mode']=='direct-external'
    image=engine/h['runtime_image'];library=engine/h['import_library']if win else image;archive=static/s['archive']
    source=tests/'tests/xui/xui_code_search_outcome_test.c';sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest()
    files=[Path(__file__).resolve(),cc,source,image,library,archive,runtime,engine/'build-report.json',static/'build-report.json',
        root/'src/xui/code/xui_code_search.c',root/'src/xui/code/xui_code_document_internal.h',root/'src/xui/code/xui_code_document.c',root/'src/xui/code/xui_code_document_owned.inl',
        engine/'xge_build_config.h',static/'xge_build_config.h',static/'xge_static_xrt_contract.h',*sorted((root/'include').glob('*.h')),
        *sorted((engine/'host-xrt').glob('*'))]
    inputs={str(f):sha(f)for f in files};out.mkdir(parents=True,exist_ok=False)
    report=dict(complete=False,passed=False,scope=__doc__,networkTests=False,shippingDelivery=False,baseline=a.baseline,unicodeBaseline=a.unicode_baseline,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    env=dict(os.environ);key='PATH'if win else'LD_LIBRARY_PATH';env[key]=os.pathsep.join([str(engine),str(runtime.parent),env.get(key,'')])
    def no_core():
        import resource
        resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    def run(name,cmd,refused=False):
        cmd=list(map(str,cmd));z=subprocess.run(cmd,env=env,cwd=out,capture_output=True,timeout=90,preexec_fn=None if win else no_core)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr);report['rows'].append(dict(name=name,command=cmd,exitCode=z.returncode,refused=refused,logSha256=sha(log)));save();print(name,z.returncode,flush=True)
        assert (z.returncode!=0)==refused,log;return z.stdout+z.stderr
    try:
        for kind,folder,lib in [('host',engine,library),('static',static,archive)]:
            flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-DXGE_XRT_HOST=1','-include',folder/'xge_build_config.h',
                *['-I'+str(v)for v in(root,root/'include',folder,engine/'host-xrt')]]
            flags+=['-DXGE_TEST_STATIC=1']if kind=='static'else['-DXGE_DLL=1','-DXUI_DLL=1']if win else[]
            systems=['-l'+n for n in s['system_libraries']]if kind=='static'else[]
            if not win:systems+=['-Wl,-rpath,'+str(engine),'-Wl,-rpath,'+str(runtime.parent),'-ldl','-lm','-pthread']
            for opt in ('O0','O2'):
                name=kind+'-'+opt;binary=out/(name+('.exe'if win else''))
                run(name+'-build',[cc,*flags,'-'+opt,source,lib,runtime,*systems,'-o',binary])
                modes=(7,)if a.unicode_baseline else range(7)if a.baseline else range(8)
                for mode in modes:
                    data=run(name+'-mode'+str(mode),[binary,mode],a.unicode_baseline or a.baseline and mode<2)
                    if a.baseline and mode<2:
                        marker=(b'Regex replacement truncated counted document',b'Plain replacement skipped after CRLF normalization')[mode]
                        assert marker in data
                        assert report['rows'][-1]['exitCode']==83+mode
                    if a.unicode_baseline:
                        assert report['rows'][-1]['exitCode']==86 and b'Plain replacement expanded partial grapheme'in data
        report.update(complete=True,inputsUnchanged=all(sha(f)==h for f,h in inputs.items()));report['passed']=report['inputsUnchanged'];assert report['passed']
    except Exception as error:report.update(complete=True,passed=False,error=str(error));raise
    finally:save()


if __name__=='__main__':main()
