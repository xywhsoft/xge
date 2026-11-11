"""Bounded legacy SVG cache reset control; stop before unsafe reentry.

The old-control assertion must fail before attempting recursive Clear. Corrected
code preserves a new entry published by an invalidator. Aggregate-only CPU
check, not full SDK qualification; no devices, files, window or network.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('root','test-root','static','runtime','out'):
        parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--old-control',action='store_true');args=parser.parse_args()
    root,tests,static,runtime,out=[p.resolve()for p in(args.root,args.test_root,args.static,args.runtime,args.out)]
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    metadata=json.loads((static/'build-report.json').read_text())
    assert metadata['passed']and metadata['sourceUnchanged']and Path(metadata['source_root']).resolve()==root
    assert all(sha(p)==h for p,h in metadata['inputs'].items())
    compiler=Path(shutil.which('gcc')).resolve();source=tests/'tests/xge/xge_svg_cache_reset_control.c';archive=static/metadata['archive']
    inputs=dict(metadata['inputs']);inputs.update({str(p):sha(p)for p in(Path(__file__).resolve(),source,compiler,archive,runtime,static/'build-report.json',static/'xge_build_config.h')})
    out.mkdir(parents=True,exist_ok=False);env=dict(os.environ)
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(passed=False,scope=__doc__,oldControl=args.old_control,shippingDelivery=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,command,reject=False):
        command=list(map(str,command));z=subprocess.run(command,cwd=root,env=env,capture_output=True,timeout=240)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        report['rows'].append(dict(name=name,command=command,expectedFailure=reject,exitCode=z.returncode,logSha256=sha(log)))
        save();print(name,z.returncode,flush=True);assert bool(z.returncode)==reject,log
        if reject:assert b'g_xgeSvgCacheHead==NULL'in z.stdout+z.stderr,log
    flags=[v for v in metadata['flags']if v!='-O2'and not v.startswith('-DXGE_BUILD_ABI_FINGERPRINT=')]
    flags+=['-include',static/'xge_build_config.h'];systems=['-l'+v for v in metadata['system_libraries']]
    if os.name!='nt':systems+=['-pthread','-ldl','-lm','-Wl,-rpath,'+str(runtime.parent)]
    member=next(Path(row['object']).name for row in metadata['objects']if row['source'].replace('\\','/').endswith('single/xge.c'))
    for level in ('O0','O2'):
        binary=out/(level+('.exe'if os.name=='nt'else''));linkmap=out/(level+'.map')
        run(level+'-build',[compiler,*flags,'-'+level,source,archive,runtime,*systems,'-Wl,--gc-sections','-Wl,-Map,'+str(linkmap),'-o',binary])
        assert '('+member+')'not in linkmap.read_text(errors='replace')
        run(level+'-run',[binary],args.old_control)
    report.update(passed=True,inputsUnchanged=all(sha(p)==h for p,h in inputs.items()));assert report['inputsUnchanged'];save()


if __name__=='__main__':main()
