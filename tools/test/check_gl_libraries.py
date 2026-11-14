"""Private real typed GL table/library owner gate, no rendering/device claim."""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root',type=Path,required=True)
    p.add_argument('--out',type=Path,required=True)
    a=p.parse_args();root,out=a.root.resolve(),a.out.resolve()
    cc,cpp=[Path(shutil.which(n)).resolve()for n in ('gcc','g++')]
    source=root/'tests/xge/xge_gl_library_test.c'
    files=[Path(__file__).resolve(),source,cc,cpp,*[root/'src/xge/render'/n for n in ('xge_gl.h','xge_gl_slots.inc','xge_gl_library.h')]]
    sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest()
    inputs={str(f):sha(f)for f in files};out.mkdir(parents=True,exist_ok=False)
    report=dict(complete=False,passed=False,scope=__doc__,networkTests=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    try:
        for three,shadow in ((0,0),(1,0),(1,1)):
            for standard,compiler,language in (('c11',cc,'c'),('c++11',cpp,'c++'),('c++17',cpp,'c++')):
                for opt in ('O0','O2'):
                    name=f'3d{three}-shadow{shadow}-{standard}-{opt}'
                    binary=out/(name+('.exe'if os.name=='nt'else''))
                    commands=[('build',[compiler,'-std='+standard,'-x',language,'-'+opt,'-Wall','-Wextra','-Werror','-Wno-cast-function-type',f'-DXGE_ENABLE_3D={three}',f'-DXGE3D_ENABLE_SHADOW={shadow}','-I'+str(root/'src/xge/render'),source,'-o',binary]),('run',[binary])]
                    for kind,cmd in commands:
                        cmd=list(map(str,cmd));z=subprocess.run(cmd,capture_output=True,timeout=60)
                        log=out/(name+'-'+kind+'.log');log.write_bytes(z.stdout+z.stderr)
                        report['rows'].append(dict(name=name+'-'+kind,command=cmd,exitCode=z.returncode,logSha256=sha(log)))
                        save();assert not z.returncode,log
        report.update(complete=True,passed=True,inputsUnchanged=all(sha(f)==h for f,h in inputs.items()))
        assert report['inputsUnchanged']
    except Exception as error:report.update(complete=True,passed=False,error=str(error));raise
    finally:save()


if __name__=='__main__':main()
