"""Production graphics ownership admission: phases, rollback and concurrent close."""
from pathlib import Path
import argparse, hashlib, json, os, shutil, subprocess


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root',type=Path,default=Path(__file__).resolve().parents[2])
    p.add_argument('--out',type=Path,required=True)
    p.add_argument('--sanitize',action='store_true')
    a=p.parse_args();root,out=a.root.resolve(),a.out.resolve()
    if a.sanitize and os.name=='nt':p.error('Linux sanitizer toolchain required')
    cc=shutil.which('gcc');assert cc
    paths=[Path(__file__).resolve(),Path(cc),root/'tests/xge/xge_graphics_lifetime_test.c',
        root/'src/xge/core/xge_graphics_lifetime.h',
        root/'extlibs/xrt/xrt.h',root/'extlibs/xrt/xrt_config.h',*sorted((root/'include').glob('*.h'))]
    sha=lambda path:hashlib.sha256(Path(path).read_bytes()).hexdigest()
    inputs={str(path):sha(path) for path in paths};out.mkdir(parents=True,exist_ok=False)
    report=dict(passed=False,scope=__doc__,inputs=inputs,rows=[],sanitizers=a.sanitize)
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    for opt in ('O0','O2'):
        binary=out/('graphics-lifetime-'+opt+('.exe' if os.name=='nt' else ''))
        command=[cc,'-std=c11','-Wall','-Wextra','-'+opt,'-I'+str(root),'-I'+str(root/'include'),
            str(root/'tests/xge/xge_graphics_lifetime_test.c'),'-o',str(binary)]
        if os.name!='nt':command+=['-lm','-pthread']
        if a.sanitize:command[1:1]=['-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']
        for step,argv in (('build',command),('run',[str(binary)])):
            env=os.environ.copy()
            if a.sanitize:env['ASAN_OPTIONS']='detect_leaks=1:detect_stack_use_after_return=1'
            r=subprocess.run(argv,cwd=root,env=env,capture_output=True,timeout=180)
            log=out/(opt+'-'+step+'.log');log.write_bytes(r.stdout+r.stderr)
            report['rows'].append(dict(optimization=opt,step=step,command=argv,exitCode=r.returncode,logSha256=sha(log)))
            save()
            if r.returncode:raise RuntimeError(str(log))
    assert all(sha(path)==digest for path,digest in inputs.items())
    report.update(passed=True,inputsUnchanged=True);save()
    print(json.dumps(dict(passed=True,rows=len(report['rows']))))


if __name__=='__main__':main()
