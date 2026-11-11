"""PNG file/memory bounds, alpha, write errors and allocation-prefix ownership.

Use actual qualified host/static products and strict public C/C++ clients.
Fault injection additionally compiles the matching aggregate; optional Linux
UBSan covers that aggregate, not other archive members or the runtime image.
No graphics context, window, audio device or network is used.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('root','test-root','engine','static','runtime','out'):
        parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--sanitizers',action='store_true')
    args=parser.parse_args()
    root,tests,engine,static,runtime,out=[v.resolve()for v in
        (args.root,args.test_root,args.engine,args.static,args.runtime,args.out)]
    windows=os.name=='nt';assert not(windows and args.sanitizers)
    sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest()
    host=json.loads((engine/'build-report.json').read_text());native=json.loads((static/'build-report.json').read_text())
    assert host['linked']and native['complete']and native['passed']and native['sourceUnchanged']
    assert Path(host['source_root']).resolve()==Path(native['source_root']).resolve()==root
    assert all(sha(v)==h for v,h in native['inputs'].items())
    image=engine/host['runtime_image'];library=engine/host['import_library']if windows else image
    archive=static/native['archive'];public=tests/'tests/xge/xge_image_png_public_test.c'
    private=tests/'tests/xge/xge_image_png_io_test.c'
    cc,cpp=[Path(shutil.which(v)).resolve()for v in('gcc','g++')]
    files=[Path(__file__).resolve(),public,private,runtime,image,library,archive,cc,cpp,
        engine/'build-report.json',static/'build-report.json',static/'xge_build_config.h',
        *map(Path,native['inputs'])]
    inputs={str(v):sha(v)for v in files};out.mkdir(exist_ok=False,parents=True)
    report=dict(passed=False,scope=__doc__,inputs=inputs,rows=[],shippingDelivery=False)
    env=dict(os.environ)
    env['PATH']=str(engine)+os.pathsep+str(runtime.parent)+os.pathsep+env['PATH']
    if not windows:env['LD_LIBRARY_PATH']=str(engine)+os.pathsep+str(runtime.parent)+os.pathsep+env.get('LD_LIBRARY_PATH','')
    env['UBSAN_OPTIONS']='halt_on_error=1:print_stacktrace=1'
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    def run(name,command):
        command=list(map(str,command))
        try:
            result=subprocess.run(command,cwd=out,env=env,capture_output=True,timeout=240)
            code,data=result.returncode,result.stdout+result.stderr
        except subprocess.TimeoutExpired as error:
            code,data='timeout',(error.stdout or b'')+(error.stderr or b'')
        log=out/(name+'.log');log.write_bytes(data)
        report['rows'].append(dict(name=name,command=command,exitCode=code,logSha256=sha(log)))
        save();print(name,code,flush=True);assert code==0,log
        return data
    flags=[v for v in native['flags']if v not in('-O2','-std=c11')and not v.startswith('-DXGE_BUILD_ABI_FINGERPRINT=')]
    # Tests execute in the evidence directory, rather than writing PNGs into
    # the frozen source tree. Native build -I paths were relative to root.
    flags=['-I'+str(root/v[2:])if v.startswith('-I')and not Path(v[2:]).is_absolute()else v for v in flags]
    flags+=['-I'+str(root),'-I'+str(root/'include')]
    systems=['-l'+v for v in native['system_libraries']]
    if not windows:systems+=['-pthread','-ldl','-lm','-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine)]
    member=next(Path(v['object']).name for v in native['objects']if v['source'].replace('\\','/').endswith('single/xge.c'))
    for variant in ['O0','O2',*(['ubsan']if args.sanitizers else[])]:
        extra=['-g','-fsanitize=undefined','-fno-sanitize-recover=all']if variant=='ubsan'else[]
        binary=out/(variant+('.exe'if windows else''));linkmap=out/(variant+'.map')
        run(variant+'-build',[cc,*flags,'-std=c11','-include',static/'xge_build_config.h',
            '-O2'if variant=='O2'else'-O0',*extra,private,runtime,archive,*systems,
            '-Wl,--gc-sections','-Wl,-Map,'+str(linkmap),'-o',binary])
        assert '('+member+')'not in linkmap.read_text(errors='replace')
        assert b'PNG injected short-write and close failure passed'in run(variant+'-run',[binary])
    publicFlags=[v for v in flags if v!='-DXGE_STATIC_BUILD=1']
    for kind,folder,link in [('host',engine,library),('static',static,archive)]:
        defines=['-DXGE_DLL=1','-DXUI_DLL=1']if kind=='host'else['-DXGE_STATIC_BUILD=1','-DXGE_TEST_STATIC=1']
        for standard,level in [('c11','O0'),('c11','O2'),('c++11','O2'),('c++17','O2')]:
            name=kind+'-'+standard.replace('+','p')+'-'+level;binary=out/(name+('.exe'if windows else''))
            tool=cpp if standard.startswith('c++')else cc
            run(name+'-build',[tool,*publicFlags,'-std='+standard,'-'+level,'-Werror',*defines,
                '-I'+str(folder),'-I'+str(engine/'host-xrt'),'-include',folder/'xge_build_config.h',
                public,link,runtime,*systems,'-Wl,--gc-sections','-o',binary])
            data=run(name+'-run',[binary])
            assert b'PNG final-row guard and alpha roundtrip passed'in data
            assert b'Public PNG host/static bounds, errors and ownership passed'in data
    report.update(passed=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()))
    assert report['inputsUnchanged'];save()


if __name__=='__main__':main()
