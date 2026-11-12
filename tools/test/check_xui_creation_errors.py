"""Check real widget creation/error boundaries against an embedded native build.

Links the build's actual objects so native-part construction is tested without
exporting private hooks. Client callbacks run at O0 and O2. This is CPU coverage,
not GPU, sanitizer or whole-runtime heap qualification.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--engine',type=Path,required=True)
    p.add_argument('--out',type=Path,required=True)
    p.add_argument('--run-dir',type=Path)
    a=p.parse_args();engine=a.engine.resolve();out=a.out.resolve();root=Path(__file__).resolve().parents[2]
    report=json.loads((engine/'build-report.json').read_text(encoding='utf8'));assert report['linked'] and report['xrt_mode']=='embedded'
    source=Path(report['source_root']);cc=report['compiler'][0]
    objects=[engine/'obj'/(s.replace('/','_').replace('\\','_')+'.o')for s in report['sources']]
    if 'XGE_ENABLE_HARFBUZZ'in report['features']:objects.append(source/'build/shared/obj/extlibs_harfbuzz_src_harfbuzz.cc.o')
    if os.name=='nt':objects.append(engine/'obj/xge_res.o')
    assert all(o.is_file()for o in objects)
    out.mkdir(parents=True,exist_ok=False);(out/'source').mkdir();(out/'tmp').mkdir()
    runDir=a.run_dir.resolve()if a.run_dir else out;assert runDir.is_dir()
    sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest()
    pins=[Path(__file__).resolve(),Path(cc),engine/'build-report.json',*objects]
    for name in('xui_creation_error_test.c','xui_test_proxy.c','xui_test_proxy.h'):
        original=root/'tests/xui'/name;target=out/'source'/name;shutil.copy2(original,target);pins.extend((original,target))
    # These declarations must match the actual producer build. Include closure
    # stamps are checked as well as hashing the objects used by the linker.
    for obj in objects:
        metadata=obj.with_suffix('.json')
        if metadata.is_file():
            pins.append(metadata)
            if obj.name=='xge_res.o':
                pins.extend((source/'platform/windows/xge.rc',source/'res/xge/icons/xx_dll.ico'))
                continue
            for path,stamp in json.loads(metadata.read_text())['deps'].items():
                f=Path(path);f=f if f.is_absolute()else source/f;st=f.stat();assert [st.st_mtime_ns,st.st_size]==stamp,f;pins.append(f)
    inputs={str(f):sha(f)for f in pins};result=dict(passed=False,inputs=inputs,rows=[],scope=__doc__)
    def save():(out/'results.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    env=dict(os.environ,TEMP=str(out/'tmp'),TMP=str(out/'tmp'),TMPDIR=str(out/'tmp'))
    systems=['-lm','-lws2_32','-liphlpapi','-lgdi32','-luser32','-lshell32','-lole32','-loleaut32','-luuid','-limm32','-lwinmm','-lavrt','-ldwrite']if os.name=='nt'else['-lm','-lX11','-lXi','-lXcursor','-lGL','-ldl','-lpthread']
    try:
        for opt in('O0','O2'):
            flags=[f for f in report['flags']if f not in('-O0','-O1','-O2','-O3','-Os')]
            exe=out/('creation-'+opt+('.exe'if os.name=='nt'else''))
            command=[cc,*flags,'-'+opt,'-Werror','-I'+str(out/'source'),str(out/'source/xui_creation_error_test.c'),str(out/'source/xui_test_proxy.c'),*map(str,objects),*systems,'-o',str(exe)]
            build=subprocess.run(command,cwd=source,env=env,capture_output=True,timeout=180);(out/(opt+'-build.log')).write_bytes(build.stdout+build.stderr);assert not build.returncode,out/(opt+'-build.log')
            target=runDir/('.xui-creation-errors-'+str(os.getpid())+'-'+opt+exe.suffix);assert not target.exists();shutil.copy2(exe,target)
            try:
                run=subprocess.run([str(target)],cwd=out,env=env,capture_output=True,timeout=60);(out/(opt+'-run.log')).write_bytes(run.stdout+run.stderr);result['rows'].append(dict(opt=opt,build=command,command=[str(target)],exit=run.returncode,programSha256=sha(target),stdout=run.stdout.decode(errors='replace'),stderr=run.stderr.decode(errors='replace')));save();assert not run.returncode,out/(opt+'-run.log');print('PASS',opt,run.stdout.decode(errors='replace').strip(),flush=True)
            finally:
                assert sha(target)==sha(exe);target.unlink()
        assert all(sha(f)==h for f,h in inputs.items());result['passed']=True
    finally:save()

if __name__=='__main__':main()
