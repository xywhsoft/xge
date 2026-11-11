"""Native Emoji references, transactional edits and retryable CPU caches.

Exercise actual host/static products through public C/C++ clients, and the
matched aggregate for private invariants. Optional Linux sanitizers instrument
the aggregate; TSan also rebuilds canonical XRT, not the other archive objects.
No window, graphics/audio device, network or language delivery is exercised.
Mutable pack publication semantics are a separate, currently open contract.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','test-root','engine','static','runtime','out'):
        p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--sanitizers',action='store_true')
    a=p.parse_args();root,tests,engine,static,runtime,out=[v.resolve()for v in
        (a.root,a.test_root,a.engine,a.static,a.runtime,a.out)]
    windows=os.name=='nt';assert not(a.sanitizers and windows)
    sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest()
    host=json.loads((engine/'build-report.json').read_text());native=json.loads((static/'build-report.json').read_text())
    assert host['linked']and native['complete']and native['passed']and native['sourceUnchanged']
    assert Path(host['source_root']).resolve()==Path(native['source_root']).resolve()==root
    assert host['features']==native['features']and 'XGE_ENABLE_EMOJI'in host['features']
    assert host['xrt_host_header_sha256']==native['xrt_header_sha256']
    assert all(sha(v)==h for v,h in native['inputs'].items())
    image=engine/host['runtime_image'];hostLibrary=engine/host['import_library']if windows else image
    archive=static/native['archive'];source=tests/'tests/xge/xge_emoji_ownership_test.c';public=tests/'tests/xge/xge_emoji_public_test.c'
    cc,cpp=[Path(shutil.which(v)).resolve()for v in('gcc','g++')]
    files=[Path(__file__).resolve(),cc,cpp,source,public,image,hostLibrary,archive,runtime,
        engine/'build-report.json',static/'build-report.json',engine/'xge_build_config.h',static/'xge_build_config.h',
        static/'xge_static_xrt_contract.h',*[Path(v)for v in native['inputs']]]
    files.extend(v for v in(engine/'host-xrt').iterdir()if v.is_file())
    inputs={str(v):sha(v)for v in files};out.mkdir(parents=True,exist_ok=False);env=dict(os.environ)
    if windows:
        shutil.copy2(runtime,out/runtime.name);shutil.copy2(image,out/image.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+str(engine)+':'+env.get('LD_LIBRARY_PATH','')
    env.update(TSAN_OPTIONS='halt_on_error=1:exitcode=66',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    report=dict(passed=False,scope=__doc__,shippingDelivery=False,window=False,networkTests=False,
        sanitizers=a.sanitizers,mutablePublicationQualified=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,command,seconds=240):
        command=list(map(str,command))
        try:
            z=subprocess.run(command,cwd=root,env=env,capture_output=True,timeout=seconds)
            code=z.returncode;data=z.stdout+z.stderr
        except subprocess.TimeoutExpired as error:
            code='timeout';data=(error.stdout or b'')+(error.stderr or b'')
        log=out/(name+'.log');log.write_bytes(data);report['rows'].append(dict(name=name,command=command,exitCode=code,logSha256=sha(log)))
        save();print(name,code,flush=True);assert code==0,log
        return data
    flags=[v for v in native['flags']if v not in('-O2','-std=c11')and not v.startswith('-DXGE_BUILD_ABI_FINGERPRINT=')]
    systems=['-l'+v for v in native['system_libraries']]
    if not windows:systems+=['-pthread','-ldl','-lm','-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine)]
    member=next(Path(v['object']).name for v in native['objects']if v['source'].replace('\\','/').endswith('single/xge.c'))
    for variant in ['O0','O2',*(['tsan','ubsan']if a.sanitizers else[])]:
        extra=['-g','-fsanitize=thread','-fno-omit-frame-pointer']if variant=='tsan'else(
            ['-g','-fsanitize=undefined','-fno-sanitize-recover=all']if variant=='ubsan'else[])
        provider=runtime
        if variant=='tsan':
            unit=out/'xrt.c';unit.write_text('#define XRT_MODULE_ALL\n#define XRT_IMPLEMENTATION\n#include "extlibs/xrt/xrt.h"\n')
            inputs[str(unit)]=sha(unit);provider=out/'xrt.o'
            run('instrumented-xrt-build',[cc,*flags,'-std=c11','-include',static/'xge_build_config.h','-O0',*extra,'-c',unit,'-o',provider])
        binary=out/(variant+('.exe'if windows else''));linkmap=out/(variant+'.map')
        run(variant+'-build',[cc,*flags,'-std=c11','-include',static/'xge_build_config.h',
            '-O2'if variant=='O2'else'-O0',*extra,source,provider,archive,*systems,
            '-Wl,--gc-sections','-Wl,-Map,'+str(linkmap),'-o',binary])
        assert '('+member+')'not in linkmap.read_text(errors='replace')
        data=run(variant+'-run',[binary],90)
        assert b'Emoji native ownership, transaction and retry passed'in data
        assert b'1000 hot matches without XRT heap operations'in data
    publicFlags=[v for v in flags if v!='-DXGE_STATIC_BUILD=1']
    for kind,folder,library in [('host',engine,hostLibrary),('static',static,archive)]:
        defines=['-DXGE_DLL=1','-DXUI_DLL=1']if kind=='host'else['-DXGE_STATIC_BUILD=1','-DXGE_TEST_STATIC=1']
        for standard,level in [('c11','O0'),('c11','O2'),('c++11','O2'),('c++17','O2')]:
            name=kind+'-'+standard.replace('+','p')+'-'+level;binary=out/(name+('.exe'if windows else''))
            tool=cpp if standard.startswith('c++')else cc
            run(name+'-build',[tool,*publicFlags,'-std='+standard,'-'+level,'-Werror',*defines,
                '-I'+str(folder),'-I'+str(engine/'host-xrt'),'-include',folder/'xge_build_config.h',
                public,library,runtime,*systems,'-Wl,--gc-sections','-o',binary])
            assert b'Public Emoji host/static ABI'in run(name+'-run',[binary],90)
    report.update(passed=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()))
    assert report['inputsUnchanged'];save()


if __name__=='__main__':main()
