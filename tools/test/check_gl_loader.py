"""Private typed GL transaction tests; no device or renderer claims."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import shutil
import subprocess


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root',type=Path,required=True)
    p.add_argument('--test-root',type=Path,help='independent frozen test/tool snapshot')
    p.add_argument('--out',type=Path,required=True)
    p.add_argument('--egl',action='store_true',help='also qualify real system EGL/GLES3 contexts (Linux)')
    p.add_argument('--system-headers',action='store_true',help='also compile both system GL include orders as C/C++')
    a=p.parse_args();root,out=a.root.resolve(),a.out.resolve()
    tests=a.test_root.resolve() if a.test_root else root
    cc=Path(shutil.which(os.environ.get('CC','gcc'))).resolve()
    sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest()
    source=tests/'tests/xge/xge_gl_load_test.c'
    files=[Path(__file__).resolve(),source,cc,root/'src/xge/render/xge_gl.h',
           root/'src/xge/render/xge_gl_slots.inc']
    if a.system_headers:
        cpp=Path(shutil.which(os.environ.get('CXX','g++'))).resolve()
        files.extend((cpp,tests/'tests/xui/xge_gl_system_header_test.c',root/'extlibs/xrt/xrt.h'))
        files.extend((root/'include').glob('*.h'))
        system=cc.parent.parent/'include/GL' if os.name=='nt' else Path('/usr/include/GL')
        assert (system/'gl.h').is_file();files.extend(system.glob('*.h'))
    if a.egl:
        assert os.name!='nt','No qualified Windows EGL SDK target'
        files.append(tests/'tests/xge/xge_gl_egl_test.c')
        for folder in ('/usr/include/EGL','/usr/include/KHR'):
            files.extend(Path(folder).glob('*.h'))
        lib=subprocess.check_output([cc,'-print-file-name=libEGL.so'],text=True).strip()
        assert lib!='libEGL.so';files.extend((Path(lib),Path(lib).resolve()))
    inputs={str(f):sha(f) for f in files}
    out.mkdir(parents=True,exist_ok=False)
    report=dict(passed=False,scope=__doc__,eglDevice=a.egl,owningRendererSession=False,
                hardwarePerformance=False,networkTests=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    def run(name,args):
        args=list(map(str,args));r=subprocess.run(args,capture_output=True,cwd=out,timeout=60)
        log=out/(name+'.log');log.write_bytes(r.stdout+r.stderr)
        report['rows'].append(dict(name=name,command=args,exitCode=r.returncode,logSha256=sha(log)))
        save();assert r.returncode==0,log
    for three,shadow in ((0,0),(1,0),(1,1)):
        if a.system_headers:
            for order in (0,1):
                for standard,compiler,language in (('c11',cc,'c'),('c++11',cpp,'c++'),('c++17',cpp,'c++')):
                    run(f'headers-3d{three}-shadow{shadow}-order{order}-{standard}',[compiler,'-std='+standard,
                        '-Wall','-Wextra','-Werror','-Wno-unused-function','-fsyntax-only','-x',language,
                        f'-DXGE_ENABLE_3D={three}',f'-DXGE3D_ENABLE_SHADOW={shadow}',
                        *(['-DTEST_SYSTEM_GL_FIRST=1'] if order else []),
                        '-I'+str(root),'-I'+str(root/'include'),tests/'tests/xui/xge_gl_system_header_test.c'])
        for opt in ('O0','O2'):
            name=f'3d{three}-shadow{shadow}-{opt}'
            binary=out/(name+('.exe' if os.name=='nt' else ''))
            run(name+'-build',[cc,'-std=c11','-'+opt,'-Wall','-Wextra','-Werror',
                '-Wno-cast-function-type',f'-DXGE_ENABLE_3D={three}',f'-DXGE3D_ENABLE_SHADOW={shadow}',
                '-I'+str(root/'src/xge/render'),source,'-o',binary])
            run(name+'-run',[binary])
            if a.egl:
                device=out/(name+'-egl')
                run(name+'-egl-build',[cc,'-std=c11','-'+opt,'-Wall','-Wextra','-Werror',
                    '-Wno-cast-function-type',f'-DXGE_ENABLE_3D={three}',f'-DXGE3D_ENABLE_SHADOW={shadow}',
                    '-I'+str(root/'src/xge/render'),tests/'tests/xge/xge_gl_egl_test.c','-lEGL','-o',device])
                for mode in ('0','1'):run(name+'-egl-mode'+mode,[device,mode])
    report.update(passed=True,inputsUnchanged=all(sha(f)==v for f,v in inputs.items()))
    assert report['inputsUnchanged'];save()


if __name__=='__main__':main()
