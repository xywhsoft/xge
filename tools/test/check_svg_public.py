"""Exercise actual SVG host/static products through C11 and C++11/C++17.

No implementation inclusion, private fields, test hooks, GPU, window or network.
The matching SVG lifetime report supplies its genuine project-owned font input.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('root','test-root','engine','static','runtime','lifetime','out'):
        parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();root,tests,engine,static,runtime,lifetime,out=[p.resolve()for p in
        (args.root,args.test_root,args.engine,args.static,args.runtime,args.lifetime,args.out)]
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    host=json.loads((engine/'build-report.json').read_text());archive=json.loads((static/'build-report.json').read_text())
    proof=json.loads(lifetime.read_text());assert proof['passed'] and proof['inputsUnchanged']
    assert host['linked'] and archive['passed'] and archive['sourceUnchanged']
    assert host['features']==archive['features'] and host['xrt_host_header_sha256']==archive['xrt_header_sha256']
    assert Path(host['source_root']).resolve()==Path(archive['source_root']).resolve()==root
    assert all(sha(p)==h for p,h in archive['inputs'].items())
    document=lifetime.parent/'embedded-font.svg';assert proof['inputs'][str(document)]==sha(document)
    image=Path(host['runtime_image']);image=image if image.is_absolute()else engine/image
    hostLibrary=engine/host['import_library']if os.name=='nt'else image
    library=static/archive['archive'];source=tests/'tests/xge/xge_svg_public_test.c'
    cc=Path(shutil.which('gcc')).resolve();cpp=Path(shutil.which('g++')).resolve()
    files=[Path(__file__).resolve(),source,cc,cpp,image,hostLibrary,library,runtime,lifetime,document,root/'include/xge.h',engine/'build-report.json',static/'build-report.json',static/'xge_build_config.h',static/'xge_static_xrt_contract.h',engine/'xge_build_config.h']
    files.extend(p for p in (engine/'host-xrt').iterdir()if p.is_file())
    inputs={str(p):sha(p)for p in files};out.mkdir(parents=True,exist_ok=False);env=dict(os.environ)
    if os.name=='nt':
        shutil.copy2(image,out/image.name);shutil.copy2(runtime,out/runtime.name)
    else:env['LD_LIBRARY_PATH']=str(image.parent)+':'+str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(passed=False,scope=__doc__,shippingDelivery=False,window=False,networkTests=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,command):
        command=list(map(str,command));r=subprocess.run(command,cwd=root if name.endswith('-build')else out,env=env,capture_output=True,timeout=120)
        log=out/(name+'.log');log.write_bytes(r.stdout+r.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=r.returncode,logSha256=sha(log)));save()
        print(name,r.returncode,flush=True);assert not r.returncode,log
        if name.endswith('-run'):assert b'Actual SVG product:'in r.stdout
    flags=[v for v in archive['flags']if v not in ('-O2','-std=c11','-DXGE_STATIC_BUILD=1')and not v.startswith('-DXGE_BUILD_ABI_FINGERPRINT=')]
    systems=['-l'+name for name in archive['system_libraries']]
    if os.name!='nt':systems+=['-pthread','-ldl','-lm','-Wl,-rpath,'+str(runtime.parent)]
    for product,folder,linked in [('host',engine,hostLibrary),('static',static,library)]:
        for language,optimization in [('c11','O0'),('c11','O2'),('c++11','O2'),('c++17','O2')]:
            compiler=cpp if language.startswith('c++')else cc
            tag=product+'-'+language.replace('+','p')+'-'+optimization;exe=out/(tag+('.exe'if os.name=='nt'else''))
            defines=['-DXGE_DLL=1']if product=='host'else['-DXGE_STATIC_BUILD=1','-DXGE_TEST_STATIC=1']
            run(tag+'-build',[compiler,*flags,'-'+optimization,'-std='+language,'-Werror',*defines,'-I'+str(folder),'-I'+str(engine/'host-xrt'),'-include',folder/'xge_build_config.h',source,linked,runtime,*systems,'-Wl,--gc-sections','-o',exe])
            run(tag+'-run',[exe,document])
    report.update(passed=True,inputsUnchanged=all(sha(p)==h for p,h in inputs.items()));assert report['inputsUnchanged'];save()


if __name__=='__main__':main()
