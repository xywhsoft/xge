"""Actual owned TextLayout checked state/text and original lifetime regressions.

CPU backend owns real native Font/paint references. Uses one matched host engine
and the full compiler's sole XRT; no window, graphics or network/security test.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in('root','test-root','engine','runtime','out'):p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--legacy',action='store_true',help='Reproduce old silent scalar/NUL limitations; not acceptance of checked queries')
    a=p.parse_args();root,tests,engine,runtime,out=[v.resolve()for v in(a.root,a.test_root,a.engine,a.runtime,a.out)]
    win=os.name=='nt';meta=json.loads((engine/'build-report.json').read_text(encoding='utf-8'))
    assert meta['xrt_mode']=='host'and Path(meta['source_root']).resolve()==root
    cc=Path(shutil.which('gcc')).resolve();image=engine/meta['runtime_image'];library=engine/meta['import_library']if win else image
    source=tests/'tests/xui/xui_text_layout_queries_test.c';proxy=root/'tests/xui/xui_test_proxy.c'
    sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest()
    files=[Path(__file__).resolve(),cc,source,proxy,runtime,image,library,engine/'build-report.json',
        root/'src/xui/core/xui_text.c',root/'include/xui.h',
        *[source.parent/n for n in('xui_owned_layout_test.c','xui_owned_fonts_test.c','xui_test_proxy.h')],
        *sorted((engine/'host-xrt').glob('*'))]
    if not a.legacy:files.append(root/'src/xui/core/xui_text_queries.inl')
    inputs={str(v):sha(v)for v in files if v.is_file()};out.mkdir(parents=True,exist_ok=False)
    snapshots=out/'input-snapshots';snapshots.mkdir()
    for v in[Path(__file__).resolve(),source]:shutil.copy2(v,snapshots/v.name)
    report=dict(complete=False,passed=False,scope=__doc__,legacyLimitReproduction=a.legacy,
        shippingDelivery=False,networkTests=False,inputs=inputs,rows=[])
    env=dict(os.environ);key='PATH'if win else'LD_LIBRARY_PATH';env[key]=os.pathsep.join([str(engine),str(runtime.parent),env.get(key,'')])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    def run(name,args):
        args=list(map(str,args));z=subprocess.run(args,env=env,cwd=out,capture_output=True,timeout=180)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        report['rows'].append(dict(name=name,command=args,exitCode=z.returncode,logSha256=sha(log)));save();print(name,z.returncode,flush=True);assert not z.returncode,log
        return z.stdout+z.stderr
    try:
        flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-parameter','-Wno-cast-function-type',
            '-DXGE_DLL=1','-DXUI_DLL=1','-DXGE_XRT_HOST=1','-include',engine/'xge_build_config.h',
            *['-I'+str(v)for v in(root,root/'include',root/'extlibs/xrt',engine,engine/'host-xrt')]]
        if not a.legacy:flags+=['-DXUI_TEST_LAYOUT_QUERIES_FIXED=1']
        systems=['-l'+v for v in (['ws2_32','iphlpapi','shell32','ole32','advapi32','user32']if win else['m','dl','pthread'])]
        for level in('O0','O2'):
            binary=out/(level+('.exe'if win else''))
            run(level+'-build',[cc,*flags,'-'+level,source,proxy,library,runtime,*systems,*([]if win else['-Wl,-rpath,'+str(engine),'-Wl,-rpath,'+str(runtime.parent)]),'-o',binary])
            output=run(level+'-run',[binary]);assert (b'Legacy layout limits reproduced:'if a.legacy else b'Layout queries: sized NUL/empty source, checked state/expired Context')in output
        report.update(complete=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()));report['passed']=report['inputsUnchanged'];assert report['passed']
    except Exception as e:report.update(complete=True,passed=False,error=str(e));raise
    finally:save()


if __name__=='__main__':main()
