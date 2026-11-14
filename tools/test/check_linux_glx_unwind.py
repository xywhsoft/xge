"""Vendored GLX/X11 startup faults, actual calls and healthy local-window retry.

External platform entry points are failed by ELF --wrap; no replacement engine
implementation, remote service or security/network testing. Xvfb has TCP off.
"""
from pathlib import Path
import argparse,hashlib,json,os,resource,select,shutil,signal,subprocess,time


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ['root','out']:p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--legacy-root',type=Path)
    p.add_argument('--egl',action='store_true',help='Exercise X11 EGL instead of GLX')
    p.add_argument('--gles',action='store_true',help='Exercise EGL/OpenGL ES 3 instead of desktop OpenGL')
    a=p.parse_args();a.egl=a.egl or a.gles;assert os.name!='nt';root=a.root.resolve();out=a.out.resolve();out.mkdir(parents=True,exist_ok=False)
    cc=Path(shutil.which('gcc')).resolve();cpp=Path(shutil.which('g++')).resolve();xvfb=Path(shutil.which('Xvfb')).resolve()
    source=root/'tests/xge/sokol_linux_glx_failure_test.c';fault=root/'tests/xge/linux_startup_faults.c'
    files=[Path(__file__).resolve(),source,fault,root/'tests/xge/linux_startup_faults.h',root/'extlibs/sokol/sokol_app.h',cc,cpp,xvfb]
    if a.egl:files.append(root/'tests/xge/linux_egl_faults.c')
    if a.legacy_root:files.append(a.legacy_root/'extlibs/sokol/sokol_app.h')
    sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest();inputs={str(v):sha(v)for v in files}
    snapshots=out/'input-snapshots';snapshots.mkdir()
    for v in [p for p in files if p.suffix in ['.c','.h','.py']]:shutil.copy2(v,snapshots/v.name)
    r=dict(complete=False,passed=False,scope=__doc__,egl=a.egl,gles=a.gles,shippingDelivery=False,networkTests=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf-8')
    def no_core():resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    env=dict(os.environ);env['LIBGL_ALWAYS_SOFTWARE']='1'
    # Keep the local server alive between clients; an automatic last-client
    # reset can temporarily reject XOpenDisplay before the chosen EGL stage.
    log=(out/'xvfb.log').open('wb');server=subprocess.Popen([xvfb,'-displayfd','1','-screen','0','640x480x24','-nolisten','tcp','-noreset'],stdout=subprocess.PIPE,stderr=log)
    try:
        assert select.select([server.stdout],[],[],15)[0],'Xvfb display handshake timed out'
        number=server.stdout.readline().decode().strip();assert number.isdigit(),number;env['DISPLAY']=':'+number
        def run(name,command,expected=0):
            command=list(map(str,command));t=time.monotonic();z=subprocess.run(command,cwd=out,env=env,capture_output=True,timeout=90,preexec_fn=no_core)
            f=out/(name+'.log');f.write_bytes(z.stdout+z.stderr);r['rows'].append(dict(name=name,command=command,exitCode=z.returncode,expectedExitCode=expected,seconds=time.monotonic()-t,logSha256=sha(f)));save();print(name,z.returncode,flush=True);assert z.returncode==expected,f
        flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-parameter','-Wno-cast-function-type']
        wrappers=['dlopen','dlclose','dlsym','XOpenDisplay','XCloseDisplay','XCreateColormap','XFreeColormap','XCreateWindow','XDestroyWindow','XAllocSizeHints','XGetVisualInfo']
        systems=['-lX11','-lXi','-lXcursor','-lGL','-ldl','-lm','-pthread']
        modes=list(range(1,19));extra=[];sources=[source,fault]
        if a.egl:
            modes=list(range(14,18))+list(range(101,113));extra=['-DXGE_TEST_EGL'];systems.append('-lEGL');sources.append(root/'tests/xge/linux_egl_faults.c')
            wrappers+=['eglBindAPI','eglGetDisplay','eglInitialize','eglTerminate','eglChooseConfig','eglGetConfigAttrib','eglCreateWindowSurface','eglDestroySurface','eglCreateContext','eglDestroyContext','eglMakeCurrent']
        if a.gles:
            extra.append('-DSOKOL_GLES3');systems.remove('-lGL');systems.append('-lGLESv2')
        for level in ['O0','O2']:
            binary=out/level;run(level+'-build',[cc,*flags,'-'+level,*extra,'-I'+str(root),*sources,*['-Wl,--wrap='+n for n in wrappers],*systems,'-o',binary])
            for mode in modes:run(level+'-stage-'+str(mode),[binary,mode])
            if a.legacy_root:
                binary=out/('legacy-'+level);run('legacy-'+level+'-build',[cc,*flags,'-'+level,*extra,'-DXGE_TEST_LEGACY_SOKOL','-I'+str(a.legacy_root),*sources,*['-Wl,--wrap='+n for n in wrappers],*systems,'-o',binary])
                run('legacy-'+level+'-visual-reject',[binary,107 if a.egl else 9],-signal.SIGSEGV)
        for version in ['c++11','c++17']:
            run('syntax-'+version,[cpp,'-std='+version,'-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-parameter','-Wno-cast-function-type',*extra,'-I'+str(root),'-x','c++','-fsyntax-only',source])
        r.update(complete=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()));r['passed']=r['inputsUnchanged'];assert r['passed']
    except Exception as error:r.update(complete=True,passed=False,error=str(error));raise
    finally:
        server.terminate();server.wait(timeout=10);log.close();save()


if __name__=='__main__':main()
