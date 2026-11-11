"""SVG native paint observation, authoritative aggregate and matched dependencies.

This is an aggregate-only development check, not a production SDK qualification.
No graphics, audio device, window, resource provider, network or security tests.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,sys


def main(family='watch',modes=8,scope=__doc__):
    parser=argparse.ArgumentParser(description=scope)
    for name in ('root','static','runtime','out'):
        parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();root,static,runtime,out=[p.resolve()for p in
        (args.root,args.static,args.runtime,args.out)]
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    metadata=json.loads((static/'build-report.json').read_text())
    assert metadata['complete']and metadata['passed']and metadata['sourceUnchanged']
    previous=Path(metadata['source_root']);archive=static/metadata['archive']
    compiler=Path(shutil.which('gcc')).resolve();fixture=root/('tests/xge/xge_svg_'+family+'_test.c')
    inputs={str(p):sha(p)for p in (Path(__file__).resolve(),Path(sys.argv[0]).resolve(),compiler,fixture,archive,runtime,
        static/'build-report.json',static/'xge_build_config.h')}
    for p,h in metadata['inputs'].items():
        path=Path(p)
        if path.is_relative_to(previous):
            current=root/path.relative_to(previous)
            if current.is_file():inputs[str(current)]=sha(current)
    out.mkdir(parents=True,exist_ok=False)
    report=dict(passed=False,fullNativeRebuild=False,shippingDelivery=False,scope=scope,inputs=inputs,rows=[])
    env=dict(os.environ)
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,command):
        command=list(map(str,command));z=subprocess.run(command,cwd=root,env=env,capture_output=True,timeout=240)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=z.returncode,logSha256=sha(log)))
        save();print(name,z.returncode,flush=True);assert not z.returncode,log
    # Exact current source first. The matched remaining archive members keep
    # the previous qualified source/ABI; its old aggregate must not be linked.
    flags=[v.replace(str(previous),str(root)).replace(previous.as_posix(),root.as_posix())
        for v in metadata['flags']if v!='-O2'and not v.startswith('-DXGE_BUILD_ABI_FINGERPRINT=')]
    flags+=['-include',static/'xge_build_config.h']
    systems=['-l'+name for name in metadata['system_libraries']]
    if os.name!='nt':systems+=['-pthread','-ldl','-lm','-Wl,-rpath,'+str(runtime.parent)]
    member=next(Path(row['object']).name for row in metadata['objects']if row['source'].replace('\\','/').endswith('single/xge.c'))
    for level in ('O0','O2'):
        binary=out/(level+('.exe'if os.name=='nt'else''));linkmap=out/(level+'.map')
        run(level+'-build',[compiler,*flags,'-'+level,fixture,archive,runtime,*systems,
            '-Wl,--gc-sections','-Wl,-Map,'+str(linkmap),'-o',binary])
        assert '('+member+')'not in linkmap.read_text(errors='replace')
        for mode in range(modes):run(level+'-'+str(mode),[binary,mode])
    report.update(passed=True,inputsUnchanged=all(sha(p)==h for p,h in inputs.items()))
    assert report['inputsUnchanged'];save()


if __name__=='__main__':main()
