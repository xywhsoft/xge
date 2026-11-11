"""Shared CPU FontFace lifetime, outlines, fallback replacement and OOM.

Normal O0/O2 tests use the supplied host XRT. Optional Linux TSan compiles the
same canonical XRT implementation with instrumentation, including its allocator
and synchronization; remaining native archive objects stay uninstrumented.
This gate does not install or qualify a shipping language/IDE delivery.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('root','static','runtime','out'):parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--test-root',type=Path,help='Separate newer test fixture, compiled against the unchanged native root')
    parser.add_argument('--tsan',action='store_true');args=parser.parse_args()
    root,static,runtime,out=[p.resolve()for p in(args.root,args.static,args.runtime,args.out)]
    assert not args.tsan or os.name!='nt'
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    native=json.loads((static/'build-report.json').read_text())
    assert native['complete'] and native['passed'] and native['sourceUnchanged']
    assert Path(native['source_root']).resolve()==root
    assert all(sha(p)==h for p,h in native['inputs'].items())
    cc=Path(shutil.which('gcc')).resolve();archive=static/native['archive']
    tests=args.test_root.resolve()if args.test_root else root
    source=tests/'tests/xge/xge_shared_font_face_test.c';font=tests/'tests/xge/data/xge_ascii_context_fixture.ttf'
    paths=[Path(__file__).resolve(),cc,source,font,archive,runtime,static/'build-report.json',static/'xge_build_config.h',*[Path(p)for p in native['inputs']]]
    inputs={str(p):sha(p)for p in paths};out.mkdir(parents=True,exist_ok=False)
    env=dict(os.environ)
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    env['TSAN_OPTIONS']='halt_on_error=1:exitcode=66'
    report=dict(passed=False,shippingDelivery=False,fullNativeBuild=native['complete'],scope=__doc__,tsan=args.tsan,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,command,timeout=300):
        command=list(map(str,command));z=subprocess.run(command,cwd=root,env=env,capture_output=True,timeout=timeout)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=z.returncode,logSha256=sha(log)))
        save();print(name,z.returncode,flush=True);assert not z.returncode,log
        return z.stdout+z.stderr
    flags=[v for v in native['flags']if v!='-O2'and not v.startswith('-DXGE_BUILD_ABI_FINGERPRINT=')]
    systems=['-l'+v for v in native['system_libraries']]
    if os.name!='nt':systems+=['-pthread','-ldl','-lm']
    member=next(Path(v['object']).name for v in native['objects']if v['source'].replace('\\','/').endswith('single/xge.c'))
    for variant in ['O0','O2',*(['tsan']if args.tsan else[])]:
        instrumented=variant=='tsan';extra=['-g','-fsanitize=thread','-fno-omit-frame-pointer']if instrumented else[]
        provider=runtime
        if instrumented:
            unit=out/'xrt.c';unit.write_text('#define XRT_MODULE_ALL\n#define XRT_IMPLEMENTATION\n#include "extlibs/xrt/xrt.h"\n')
            inputs[str(unit)]=sha(unit);provider=out/'xrt.o'
            run('instrumented-xrt-build',[cc,*flags,'-include',static/'xge_build_config.h','-O0',*extra,'-c',unit,'-o',provider])
        binary=out/(variant+('.exe'if os.name=='nt'else''));linkmap=out/(variant+'.map')
        run(variant+'-build',[cc,*flags,'-include',static/'xge_build_config.h','-O0'if instrumented else'-'+variant,*extra,
            source,provider,archive,*systems,'-Wl,--gc-sections','-Wl,-Map,'+str(linkmap),
            *(['-Wl,-rpath,'+str(runtime.parent)]if os.name!='nt'and not instrumented else[]),'-o',binary])
        assert '('+member+')'not in linkmap.read_text(errors='replace')
        raw=run(variant+'-run',[binary,font],90)
        assert b'Shared Face and fallback passed' in raw and b'Fallback prefixes:' in raw
    report.update(passed=True,inputsUnchanged=all(sha(p)==h for p,h in inputs.items()))
    assert report['inputsUnchanged'];save()


if __name__=='__main__':main()
