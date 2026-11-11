"""SVG native reentry/lifetime with real project-owned embedded fonts.

Uses the authoritative aggregate plus matched remaining archive objects. Old
controls stop before dereferencing invalidated paint/document pointers. No GPU,
window, audio device, network or external font is used.
"""
from pathlib import Path
import argparse,base64,hashlib,json,os,re,shutil,subprocess


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('root','test-root','static','runtime','host-header','out'):
        parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--baseline',action='store_true')
    args=parser.parse_args()
    root,tests,static,runtime,header,out=[p.resolve() for p in
        (args.root,args.test_root,args.static,args.runtime,args.host_header,args.out)]
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    canonical=lambda p:hashlib.sha256(Path(p).read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    native=json.loads((static/'build-report.json').read_text())
    assert native['complete'] and native['passed'] and native['sourceUnchanged']
    assert native['xrt_header_sha256']==canonical(header)==canonical(root/'extlibs/xrt/xrt.h')
    if not args.baseline:
        assert Path(native['source_root']).resolve()==root
        assert all(sha(p)==h for p,h in native['inputs'].items())
    source=tests/'tests/xge/xge_svg_lifetime_test.c'
    font=tests/'tests/xge/data/xge_ascii_context_fixture.ttf'
    compiler=Path(shutil.which('gcc')).resolve();archive=static/native['archive']
    files=[Path(__file__).resolve(),source,font,compiler,archive,runtime,header,static/'build-report.json',static/'xge_build_config.h']
    files.extend(root/p for p in ('single/xge.c','include/xge.h','src/xge/render/xge_svg.c'))
    inputs={str(p):sha(p)for p in files};out.mkdir(parents=True,exist_ok=False);env=dict(os.environ)
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    payload=base64.b64encode(font.read_bytes()).decode('ascii')
    document=out/'embedded-font.svg'
    document.write_text('<svg width="64" height="32" viewBox="0 0 64 32"><style>'+''.join(
        "@font-face { font-family: '"+name+"'; src: url('data:font/ttf;base64,"+payload+"'); }"
        for name in ('Local','Unused'))+'</style><g id="layer"><rect id="box" x="2" y="3" width="12" height="10"/><circle cx="30" cy="16" r="8"/></g></svg>',encoding='utf-8')
    inputs[str(document)]=sha(document)
    report=dict(passed=False,scope=__doc__,shippingDelivery=False,window=False,networkTests=False,baseline=args.baseline,inputs=inputs,rows=[],privateDocumentBytes={},allocationPrefixes={})
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,command,fails=False):
        command=list(map(str,command));r=subprocess.run(command,cwd=root if name.endswith('-build')else out,env=env,capture_output=True,timeout=180)
        log=out/(name+'.log');log.write_bytes(r.stdout+r.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=r.returncode,expectedFailure=fails,logSha256=sha(log)))
        save();print(name,r.returncode,flush=True);assert bool(r.returncode)==fails,log
        return r.stdout+r.stderr
    flags=[v for v in native['flags']if v!='-O2' and not v.startswith('-DXGE_BUILD_ABI_FINGERPRINT=')]
    flags+=['-include',static/'xge_build_config.h']
    if args.baseline:flags+=['-DXGE_TEST_SVG_LIFETIME_BASELINE=1']
    systems=['-l'+v for v in native['system_libraries']]
    if os.name!='nt':systems+=['-pthread','-ldl','-lm','-Wl,-rpath,'+str(runtime.parent)]
    member=next(Path(row['object']).name for row in native['objects']if row['source'].replace('\\','/').endswith('single/xge.c'))
    for optimization in ('O0','O2'):
        exe=out/(optimization+('.exe'if os.name=='nt'else''));linkmap=out/(optimization+'.map')
        run(optimization+'-build',[compiler,*flags,'-'+optimization,source,archive,runtime,*systems,'-Wl,--gc-sections','-Wl,-Map,'+str(linkmap),'-o',exe])
        assert '('+member+')'not in linkmap.read_text(errors='replace')
        for mode in range(7)if args.baseline else range(17):
            text=run(optimization+'-'+str(mode),[exe,mode,document],args.baseline)
            if args.baseline:
                expected=[b'a->LiveCount==b->LiveCount',b'xgeSvgAddRef(svg)==XGE_ERROR_INVALID_STATE',b'xgeSvgClear(state->svg)==XGE_ERROR_BUSY',b'xgeSvgLoadMemory(state->svg,document,documentBytes)==XGE_ERROR_BUSY',b'state->svg->iRefCount>=2',b'xgeSvgClear(state->svg)==XGE_ERROR_BUSY',b'xgeSvgPaintChildGetCount(picture,&count)==XGE_OK && count==1'][mode]
                assert expected in text
            else:
                size=int(re.search(rb'private document (\d+) bytes',text)[1])
                assert report['privateDocumentBytes'].setdefault(optimization,size)==size
                if mode in (8,13):
                    match=re.search(rb'allocation prefixes: (\d+) failed, (\d+) passed',text)
                    assert match
                    report['allocationPrefixes'][optimization+'-'+str(mode)]=dict(failures=int(match[1]),successes=int(match[2]))
        if args.baseline:
            text=run(optimization+'-layout',[exe,15,document])
            report['privateDocumentBytes'][optimization]=int(re.search(rb'private document (\d+) bytes',text)[1])
    report.update(passed=not args.baseline,baselineControlsVerified=args.baseline,inputsUnchanged=all(sha(p)==h for p,h in inputs.items()))
    assert report['inputsUnchanged'];save()


if __name__=='__main__':main()
