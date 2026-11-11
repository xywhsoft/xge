"""Real no-device audio ownership checks using the authoritative aggregate.

Private setup reaches miniaudio's CPU engine, then executes production XGE.
Remaining TUs come from the matched complete static SDK. This is a private
aggregate executable, not a replacement for dynamic SDK/language acceptance.
--baseline verifies three old assertion failures without touching freed nodes.
"""
from pathlib import Path
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import time


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('root','test-root','static','runtime','host-header','out'):
        parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--baseline',action='store_true')
    parser.add_argument('--graphics-coupling-baseline',action='store_true')
    parser.add_argument('--allocator-baseline',action='store_true')
    parser.add_argument('--resident-oom',action='store_true',help='Keep the same audio engine alive across allocation prefixes')
    parser.add_argument('--resident-oom-baseline',action='store_true')
    args=parser.parse_args()
    assert sum((args.baseline,args.graphics_coupling_baseline,args.allocator_baseline,args.resident_oom_baseline))<=1
    control=args.baseline or args.graphics_coupling_baseline or args.allocator_baseline or args.resident_oom_baseline
    root,tests,static,runtime,header,out=[value.resolve() for value in
        (args.root,args.test_root,args.static,args.runtime,args.host_header,args.out)]
    sha=lambda path:hashlib.sha256(Path(path).read_bytes()).hexdigest()
    canonical=lambda path:hashlib.sha256(Path(path).read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    archive=json.loads((static/'build-report.json').read_text())
    assert archive['complete'] and archive['passed'] and archive['sourceUnchanged']
    assert archive['xrt_header_sha256']==canonical(header)==canonical(root/'extlibs/xrt/xrt.h')
    if not control:
        # Every recorded production dependency must still be the source used
        # by this SDK, rather than a partially updated checkout.
        assert Path(archive['source_root']).resolve()==root
        assert all(sha(path)==digest for path,digest in archive['inputs'].items())
    source=tests/'tests/xge/xge_audio_lifetime_test.c'
    cc=Path(shutil.which('gcc')).resolve()
    library=static/archive['archive']
    files=[Path(__file__).resolve(),source,cc,runtime,header,library,
           static/'build-report.json',static/'xge_build_config.h']
    files.extend(root/name for name in ('single/xge.c','include/xge.h',
        'src/xge/audio/xge_audio.c','src/xge/audio/xge_audio_clip.inc',
        'src/xge/core/xge_core.c','src/xge/core/xge_impl.c',
        'src/xge/core/xge_graphics_lifetime.h'))
    inputs={str(path):sha(path) for path in files}
    out.mkdir(parents=True,exist_ok=False)
    env=dict(os.environ)
    if os.name=='nt':
        shutil.copy2(runtime,out/runtime.name)
    else:
        env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(passed=False,scope=__doc__,baseline=control,
        baselineKind='resident-file-allocation' if args.resident_oom_baseline else 'unsafe-shutdown' if args.baseline else 'recovered-allocation-diagnostic' if args.allocator_baseline else 'intermediate-graphics-coupling' if control else None,
        shippingDelivery=False,audioDevice=False,window=False,networkTests=False,
        inputs=inputs,rows=[])

    def save():
        (out/'results.json').write_text(json.dumps(report,indent=2)+'\n')

    def run(name,command,working,expected_failure=False):
        command=list(map(str,command));start=time.monotonic()
        result=subprocess.run(command,cwd=working,env=env,capture_output=True,timeout=360)
        log=out/(name+'.log');log.write_bytes(result.stdout+result.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=result.returncode,
            expectedFailure=expected_failure,seconds=time.monotonic()-start,logSha256=sha(log)))
        save();print(name,result.returncode,flush=True)
        assert bool(result.returncode)==expected_failure,log
        return result.stdout+result.stderr

    # Production disables internal asserts; the fixture explicitly reenables
    # them after including the aggregate. No test action can disappear at O2.
    flags=[value for value in archive['flags']
           if value!='-O2' and not value.startswith('-DXGE_BUILD_ABI_FINGERPRINT=')]
    flags+=['-include',static/'xge_build_config.h']
    if args.baseline:
        flags+=['-DXGE_TEST_OLD_BASELINE=1']
    if args.graphics_coupling_baseline:
        flags+=['-DXGE_TEST_GRAPHICS_COUPLING_BASELINE=1']
    if args.allocator_baseline:
        flags+=['-DXGE_TEST_ALLOCATOR_BASELINE=1']
    systems=['-l'+name for name in archive['system_libraries']]
    if os.name!='nt':
        systems+=['-pthread','-ldl','-lm','-Wl,-rpath,'+str(runtime.parent)]
    member=next(Path(row['object']).name for row in archive['objects']
                if row['source'].replace('\\','/').endswith('single/xge.c'))
    for optimization in ('O0','O2'):
        binary=out/(optimization+('.exe' if os.name=='nt' else ''))
        linkmap=out/(optimization+'.map')
        run(optimization+'-build',[cc,*flags,'-'+optimization,source,library,runtime,
            *systems,'-Wl,--gc-sections','-Wl,-Map,'+str(linkmap),'-o',binary],root)
        # The fixture provides exactly one production aggregate. Do not pull
        # another copy from the archive, which could obscure the private state.
        assert '('+member+')' not in linkmap.read_text(errors='replace')
        modes=(13,) if args.resident_oom_baseline else range(13,16) if args.resident_oom else range(3) if args.baseline else (7,9) if args.allocator_baseline else (8,) if control else range(16)
        for mode in modes:
            text=run(optimization+'-'+str(mode),[binary,str(mode)],out,control)
            if control:
                expected=b'before.LiveCount==after.LiveCount' if args.resident_oom_baseline else b'!xrtGetError()' if args.allocator_baseline else b'xgeGraphicsOwnedResourceCount()==0' if args.graphics_coupling_baseline else b'xgeAudioIsReady()' if mode<2 else b'before.LiveCount==after.LiveCount'
                assert expected in text
            elif mode==4:
                assert b'Audio ownership budgets:' in text
            elif mode==7:
                assert b'Audio file ownership budgets:' in text
    report.update(passed=not control,baselineControlsVerified=control,
                  inputsUnchanged=all(sha(path)==digest for path,digest in inputs.items()))
    assert report['inputsUnchanged'];save()


if __name__=='__main__':
    main()
