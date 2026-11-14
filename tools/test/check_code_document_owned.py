"""Actual unique CodeDocument scope, owned callback and editor graph gate.

One full compiler XRT and an exact O2 host engine. No window or network tests.
Static mode also runs the earlier unique-document observer/history/close kernel.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in('root','engine','runtime','out'):p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--static-engine',type=Path,help='Use the matched static engine, including its private kernel regressions')
    p.add_argument('--test-root',type=Path,help='Explicit test-only overlay; engine implementation stays frozen')
    a=p.parse_args();root,engine,runtime,out=[v.resolve()for v in(a.root,a.engine,a.runtime,a.out)]
    win=os.name=='nt';meta=json.loads((engine/'build-report.json').read_text(encoding='utf-8'))
    assert meta['xrt_mode']=='host'and Path(meta['source_root']).resolve()==root
    cc=Path(shutil.which('gcc')).resolve();image=engine/meta['runtime_image'];library=engine/meta['import_library']if win else image
    static=a.static_engine.resolve()if a.static_engine else None
    if static:
        static_meta=json.loads((static/'build-report.json').read_text(encoding='utf-8'))
        assert static_meta['xrt_mode']=='direct-external'and Path(static_meta['source_root']).resolve()==root
        library=static/static_meta['archive']
    source=(a.test_root.resolve()if a.test_root else root)/'tests/xui/xui_code_document_owned_callback_test.c';proxy=root/'tests/xui/xui_test_proxy.c'
    sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest()
    files=[Path(__file__).resolve(),cc,source,proxy,runtime,image,library,engine/'build-report.json',
        root/'src/xui/code/xui_code_document.c',root/'src/xui/code/xui_code_document_owned.inl',
        root/'src/xui/code/xui_code_edit.c',root/'include/xui.h',
        *[source.parent/n for n in('xui_owned_code_document_test.c','xui_owned_fonts_test.c','xui_test_proxy.h')],
        *sorted((engine/'host-xrt').glob('*'))]
    inputs={str(v):sha(v)for v in files if v.is_file()};out.mkdir(parents=True,exist_ok=False)
    if static:inputs.update({str(v):sha(v)for v in(library,static/'build-report.json')})
    snapshots=out/'input-snapshots';snapshots.mkdir()
    for v in[Path(__file__).resolve(),source]:shutil.copy2(v,snapshots/v.name)
    report=dict(complete=False,passed=False,scope=__doc__,staticEngine=bool(static),originalPrivateKernel=bool(static),shippingDelivery=False,networkTests=False,inputs=inputs,rows=[])
    env=dict(os.environ);key='PATH'if win else'LD_LIBRARY_PATH';env[key]=os.pathsep.join([str(engine),str(runtime.parent),env.get(key,'')])
    env['XUI_CODE_DOCUMENT_SAVE_TEST']=str(out/'autosave.txt')
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    def run(name,args):
        args=list(map(str,args));z=subprocess.run(args,env=env,cwd=out,capture_output=True,timeout=180)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        report['rows'].append(dict(name=name,command=args,exitCode=z.returncode,logSha256=sha(log)));save();print(name,z.returncode,flush=True);assert not z.returncode,log
        return z.stdout+z.stderr
    try:
        flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-parameter','-Wno-cast-function-type',
            *(['-DXUI_TEST_STATIC_ENGINE=1','-ffunction-sections','-fdata-sections']if static else['-DXGE_DLL=1','-DXUI_DLL=1']),'-DXGE_XRT_HOST=1','-include',engine/'xge_build_config.h',
            *['-I'+str(v)for v in(root,root/'include',root/'src/xui/core',root/'extlibs/xrt',engine,engine/'host-xrt')]]
        systems=['-l'+v for v in (['ws2_32','iphlpapi','shell32','ole32','advapi32','user32']if win else['m','dl','pthread'])]
        if static:systems=['-Wl,--gc-sections',*['-l'+v for v in static_meta['system_libraries']]]
        for level in('O0','O2'):
            binary=out/(level+('.exe'if win else''))
            run(level+'-build',[cc,*flags,'-'+level,source,proxy,library,runtime,*systems,*([]if win else['-Wl,-rpath,'+str(engine),'-Wl,-rpath,'+str(runtime.parent)]),'-o',binary])
            assert b'CodeDocument owned callbacks: graph'in run(level+'-run',[binary])
        report.update(complete=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()));report['passed']=report['inputsUnchanged'];assert report['passed']
    except Exception as e:report.update(complete=True,passed=False,error=str(e));raise
    finally:save()


if __name__=='__main__':main()
