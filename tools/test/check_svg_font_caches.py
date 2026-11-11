"""SVG stable Font storage, owned GlyphRun, retryable OOM and CPU caches.

Authoritative aggregate plus matched remaining archive dependencies. This gate
does not itself qualify a shipping native SDK. No graphics/window/network.
"""
from pathlib import Path
import argparse,base64,hashlib,json,os,re,shutil,subprocess


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('root','test-root','static','runtime','out'):
        parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--baseline',action='store_true',help='Bounded old-code address assertion only')
    args=parser.parse_args();root,tests,static,runtime,out=[p.resolve()for p in
        (args.root,args.test_root,args.static,args.runtime,args.out)]
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    native=json.loads((static/'build-report.json').read_text())
    assert native['complete']and native['passed']and native['sourceUnchanged']
    parent=Path(native['source_root']);archive=static/native['archive']
    assert parent.resolve()==root
    assert all(sha(p)==h for p,h in native['inputs'].items())
    compiler=Path(shutil.which('gcc')).resolve();source=tests/'tests/xge/xge_svg_font_cache_test.c'
    font=tests/'tests/xge/data/xge_ascii_context_fixture.ttf'
    files=[Path(__file__).resolve(),compiler,source,font,archive,runtime,static/'build-report.json',static/'xge_build_config.h']
    files.extend(Path(p)for p in native['inputs'])
    inputs={str(p):sha(p)for p in files};out.mkdir(parents=True,exist_ok=False)
    payload=base64.b64encode(font.read_bytes()).decode('ascii');document=out/'embedded-font.svg'
    document.write_text('<svg width="64" height="32" viewBox="0 0 64 32"><style>'+''.join(
        "@font-face { font-family: '"+name+"'; src: url('data:font/ttf;base64,"+payload+"'); }"
        for name in ('Local','Unused'))+'</style><g id="layer"><rect id="box" x="2" y="3" width="12" height="10"/></g></svg>')
    inputs[str(document)]=sha(document)
    report=dict(passed=False,fullNativeBuild=False,shippingDelivery=False,scope=__doc__,baseline=args.baseline,
                inputs=inputs,rows=[],prefixes={},layouts={})
    env=dict(os.environ)
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,command,reject=False):
        command=list(map(str,command));result=subprocess.run(command,cwd=root,env=env,capture_output=True,timeout=240)
        log=out/(name+'.log');raw=result.stdout+result.stderr;log.write_bytes(raw)
        report['rows'].append(dict(name=name,command=command,exitCode=result.returncode,expectedFailure=reject,logSha256=sha(log)))
        save();print(name,result.returncode,flush=True);assert bool(result.returncode)==reject,log
        return raw
    flags=[v for v in native['flags']if v!='-O2'and not v.startswith('-DXGE_BUILD_ABI_FINGERPRINT=')]
    systems=['-l'+v for v in native['system_libraries']]
    if os.name!='nt':systems+=['-pthread','-ldl','-lm','-Wl,-rpath,'+str(runtime.parent)]
    member=next(Path(v['object']).name for v in native['objects']if v['source'].replace('\\','/').endswith('single/xge.c'))
    variants=('baseline',)if args.baseline else('normal','debug')
    for variant in variants:
        options=[v for v in flags if not v.startswith('-DXGE_DEBUGMODE=')]
        options+=['-DXGE_DEBUGMODE='+str(int(variant=='debug')),'-include',static/'xge_build_config.h']
        if args.baseline:options+=['-DXGE_TEST_SVG_FONT_BASELINE=1']
        for level in ('O0','O2'):
            name=variant+'-'+level;binary=out/(name+('.exe'if os.name=='nt'else''));linkmap=out/(name+'.map')
            run(name+'-build',[compiler,*options,'-'+level,source,archive,runtime,*systems,
                '-Wl,--gc-sections','-Wl,-Map,'+str(linkmap),'-o',binary])
            assert '('+member+')'not in linkmap.read_text(errors='replace')
            for mode in (range(1)if args.baseline else range(6)):
                raw=run(name+'-'+str(mode),[binary,mode,document],args.baseline)
                if args.baseline:assert b'retained Font record moved' in raw
                else:
                    layout=re.search(rb'cache entry (\d+), Font (\d+), raster (\d+) bytes',raw)
                    assert layout
                    sizes=[int(v)for v in layout.groups()]
                    assert report['layouts'].setdefault(name,sizes)==sizes
                    for key,pattern in [('cache',rb'allocation prefixes: (\d+) failed, (\d+) passed'),
                        ('text',rb'text prefixes: load (\d+) failed/(\d+) passed; bounds (\d+) failed/(\d+) passed')]:
                        match=re.search(pattern,raw)
                        if match:report['prefixes'][name+'-'+key]=[int(v)for v in match.groups()]
    report.update(passed=not args.baseline,baselineControlsVerified=args.baseline,
                  inputsUnchanged=all(sha(p)==h for p,h in inputs.items()))
    assert report['inputsUnchanged'];save()


if __name__=='__main__':main()
