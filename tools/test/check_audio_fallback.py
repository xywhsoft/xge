"""No-device fallback lifecycle, immutable path snapshots and real PCM.

Includes the authoritative production aggregate with private CPU setup; matched
static objects supply the other TUs. Old controls fail safely before any UAF.
These executables are not shipping engine SDKs or physical-device tests.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','test-root','static','runtime','host-header','out'):
        p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--baseline',action='store_true')
    a=p.parse_args();root,tests,static,runtime,header,out=[v.resolve() for v in
        (a.root,a.test_root,a.static,a.runtime,a.host_header,a.out)]
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    canonical=lambda p:hashlib.sha256(Path(p).read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    native=json.loads((static/'build-report.json').read_text())
    assert native['complete'] and native['passed'] and native['sourceUnchanged']
    assert native['xrt_header_sha256']==canonical(header)==canonical(root/'extlibs/xrt/xrt.h')
    if not a.baseline:
        assert Path(native['source_root']).resolve()==root
        assert all(sha(p)==h for p,h in native['inputs'].items())
    source=tests/'tests/xge/xge_audio_fallback_test.c';cc=Path(shutil.which('gcc')).resolve();archive=static/native['archive']
    files=[Path(__file__).resolve(),source,cc,archive,runtime,header,static/'build-report.json',static/'xge_build_config.h']
    files.extend(root/n for n in ('single/xge.c','include/xge.h','src/xge/audio/xge_audio.c','src/xge/core/xge_core.c','src/xge/core/xge_impl.c'))
    inputs={str(p):sha(p)for p in files};out.mkdir(parents=True,exist_ok=False);env=dict(os.environ)
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(passed=False,scope=__doc__,shippingDelivery=False,audioDevice=False,window=False,networkTests=False,baseline=a.baseline,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,args,fails=False):
        args=list(map(str,args));r=subprocess.run(args,cwd=root if name.endswith('-build')else out,env=env,capture_output=True,timeout=180);log=out/(name+'.log');log.write_bytes(r.stdout+r.stderr)
        report['rows'].append(dict(name=name,command=args,exitCode=r.returncode,expectedFailure=fails,logSha256=sha(log)));save();print(name,r.returncode,flush=True)
        assert bool(r.returncode)==fails,log
        return r.stdout+r.stderr
    flags=[v for v in native['flags']if v!='-O2' and not v.startswith('-DXGE_BUILD_ABI_FINGERPRINT=')]
    flags+=['-include',static/'xge_build_config.h']
    if a.baseline:flags+=['-DXGE_TEST_FALLBACK_BASELINE=1']
    systems=['-l'+v for v in native['system_libraries']]
    if os.name!='nt':systems+=['-pthread','-ldl','-lm','-Wl,-rpath,'+str(runtime.parent)]
    member=next(Path(row['object']).name for row in native['objects']if row['source'].replace('\\','/').endswith('single/xge.c'))
    for opt in ('O0','O2'):
        exe=out/(opt+('.exe'if os.name=='nt'else''));linkmap=out/(opt+'.map')
        run(opt+'-build',[cc,*flags,'-'+opt,source,archive,runtime,*systems,'-Wl,--gc-sections','-Wl,-Map,'+str(linkmap),'-o',exe])
        assert '('+member+')'not in linkmap.read_text(errors='replace')
        for mode in range(3)if a.baseline else range(10):
            text=run(opt+'-'+str(mode),[exe,mode],a.baseline)
            if a.baseline:
                expected=[b'xgeSoundFallbackGet(&sound)==XGE_OK',b'a->LiveCount==b->LiveCount',b'status==XGE_ERROR_OUT_OF_MEMORY'][mode]
                assert expected in text
    report.update(passed=not a.baseline,baselineControlsVerified=a.baseline,inputsUnchanged=all(sha(p)==h for p,h in inputs.items()))
    assert report['inputsUnchanged'];save()


if __name__=='__main__':main()
