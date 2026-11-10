"""Actual Linux static engine GL startup failure and first-frame suppression."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import shutil
import subprocess


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','test-root','static','runtime','out'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();root,tests,static,runtime,out=[v.resolve() for v in (a.root,a.test_root,a.static,a.runtime,a.out)]
    assert os.name!='nt','Linux ELF symbol wrapping only'
    meta=json.loads((static/'build-report.json').read_text())
    assert meta['passed'] and meta['sourceUnchanged'] and meta['xrt_mode']=='direct-external'
    cc=Path(shutil.which('gcc')).resolve();source=tests/'tests/xge/xge_gl_startup_test.c';archive=static/meta['archive']
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    files=[Path(__file__).resolve(),source,cc,archive,runtime,static/'build-report.json',
        static/'xge_build_config.h',static/'xge_static_xrt_contract.h',root/'extlibs/xrt/xrt.h']
    files.extend((root/'include').glob('*.h'));inputs={str(v):sha(v) for v in files}
    out.mkdir(parents=True,exist_ok=False);env=dict(os.environ)
    env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(passed=False,scope=__doc__,graphicsDevice=True,networkTests=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    def run(name,args):
        args=list(map(str,args));r=subprocess.run(args,cwd=out,env=env,capture_output=True,timeout=60)
        log=out/(name+'.log');log.write_bytes(r.stdout+r.stderr)
        report['rows'].append(dict(name=name,command=args,exitCode=r.returncode,logSha256=sha(log)))
        save();print(name,r.returncode,flush=True);assert not r.returncode,log
    for opt in ('O0','O2'):
        binary=out/opt
        run(opt+'-build',[cc,'-std=c11','-'+opt,'-Wall','-Wextra','-Werror','-Wno-cast-function-type',
            '-include',static/'xge_build_config.h',*['-I'+str(v) for v in (root,root/'include',static)],
            source,'-Wl,--wrap=dlsym',archive,runtime,*['-l'+v for v in meta['system_libraries']],
            '-Wl,-rpath,'+str(runtime.parent),'-o',binary])
        for mode in ('0','1'):run(opt+'-mode'+mode,[binary,mode])
    report.update(passed=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()))
    assert report['inputsUnchanged'];save()


if __name__=='__main__':main()
