"""Check production mesh storage rollback, self-alias updates and index bounds.

Uses deterministic allocator failures, without initializing a GPU or window.
"""
from pathlib import Path
import argparse
import hashlib
import json
import os
import shutil
import subprocess


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root',type=Path,default=Path(__file__).resolve().parents[2])
    parser.add_argument('--out',type=Path,required=True)
    parser.add_argument('--sanitize',action='store_true')
    args=parser.parse_args();root=args.root.resolve();out=args.out.resolve()
    if args.sanitize and os.name=='nt':parser.error('Use the Linux sanitizer toolchain for this gate')
    compiler=shutil.which('gcc')
    if not compiler:parser.error('GCC is required')
    files=[Path(__file__).resolve(),Path(compiler),root/'tests/xge/xge_mesh_cpu_test.c',
           root/'src/xge/render/xge_mesh.c',root/'src/xge/render/xge_mesh_cpu.inl',
           root/'extlibs/xrt/xrt.h',root/'extlibs/xrt/xrt_config.h',*sorted((root/'include').glob('*.h'))]
    sha=lambda path:hashlib.sha256(path.read_bytes()).hexdigest()
    inputs={str(path):sha(path) for path in files};out.mkdir(parents=True,exist_ok=False);rows=[]
    for optimization in ('O0','O2'):
        binary=out/('mesh-'+optimization+('.exe' if os.name=='nt' else ''))
        command=[compiler,'-std=c11','-Wall','-Wextra','-Werror','-'+optimization,
                 '-I'+str(root),'-I'+str(root/'include'),str(root/'tests/xge/xge_mesh_cpu_test.c'),'-o',str(binary)]
        if args.sanitize:command[1:1]=['-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']
        for name,argv in (('build',command),('run',[str(binary)])):
            env=dict(os.environ)
            if args.sanitize:env['ASAN_OPTIONS']='detect_leaks=1:detect_stack_use_after_return=1'
            result=subprocess.run(argv,cwd=root,env=env,capture_output=True,timeout=120)
            (out/(optimization+'-'+name+'.log')).write_bytes(result.stdout+result.stderr)
            rows.append(dict(optimization=optimization,step=name,command=argv,exitCode=result.returncode))
            if result.returncode:raise RuntimeError(optimization+' '+name+' failed: '+str(out))
    assert all(sha(Path(path))==expected for path,expected in inputs.items())
    report=dict(passed=True,inputsUnchanged=True,inputs=inputs,rows=rows,
                scope=__doc__,sanitizers=args.sanitize,fullEngineRebuild=False)
    (out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(dict(passed=True,rows=len(rows),sanitizers=args.sanitize)))


if __name__=='__main__':main()
