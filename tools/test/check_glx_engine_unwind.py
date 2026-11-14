"""Actual direct-XRT static XGE GLX failure stages and same-process window retry.

One full runtime; ELF platform-call wrapping, no replacement engine TU. Local
Xvfb software graphics with TCP disabled; no network/security tests.
"""
from pathlib import Path
import argparse,hashlib,json,os,resource,select,shutil,subprocess,time


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ['root','test-root','static','runtime','out']:p.add_argument('--'+n,type=Path,required=True)
    a=p.parse_args();assert os.name!='nt';root,tests,static,runtime,out=[v.resolve()for v in(a.root,a.test_root,a.static,a.runtime,a.out)]
    meta=json.loads((static/'build-report.json').read_text(encoding='utf-8'));assert meta['passed']and meta['sourceUnchanged']and meta['xrt_mode']=='direct-external'
    cc=Path(shutil.which('gcc')).resolve();xvfb=Path(shutil.which('Xvfb')).resolve();archive=static/meta['archive']
    sources=[tests/'tests/xge/xge_linux_glx_failure_test.c',tests/'tests/xge/linux_startup_faults.c'];sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest()
    files=[Path(__file__).resolve(),*sources,tests/'tests/xge/linux_startup_faults.h',cc,xvfb,archive,runtime,static/'build-report.json',
        static/'xge_build_config.h',static/'xge_static_xrt_contract.h',root/'extlibs/sokol/sokol_app.h',root/'extlibs/xrt/xrt.h',*sorted((root/'include').glob('*.h'))]
    inputs={str(v):sha(v)for v in files};out.mkdir(parents=True,exist_ok=False);snapshots=out/'input-snapshots';snapshots.mkdir()
    for f in [Path(__file__).resolve(),*sources,tests/'tests/xge/linux_startup_faults.h']:shutil.copy2(f,snapshots/f.name)
    r=dict(complete=False,passed=False,scope=__doc__,shippingDelivery=False,networkTests=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf-8')
    env=dict(os.environ,LIBGL_ALWAYS_SOFTWARE='1');env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    def no_core():resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    log=(out/'xvfb.log').open('wb');server=subprocess.Popen([xvfb,'-displayfd','1','-screen','0','640x480x24','-nolisten','tcp','-noreset'],stdout=subprocess.PIPE,stderr=log)
    try:
        assert select.select([server.stdout],[],[],15)[0];number=server.stdout.readline().decode().strip();assert number.isdigit();env['DISPLAY']=':'+number
        def run(name,command):
            command=list(map(str,command));t=time.monotonic();z=subprocess.run(command,cwd=out,env=env,capture_output=True,timeout=90,preexec_fn=no_core)
            f=out/(name+'.log');f.write_bytes(z.stdout+z.stderr);r['rows'].append(dict(name=name,command=command,exitCode=z.returncode,seconds=time.monotonic()-t,logSha256=sha(f)));save();print(name,z.returncode,flush=True);assert not z.returncode,f
        flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-include',static/'xge_build_config.h',*['-I'+str(v)for v in(root,root/'include',static)]]
        wraps=['dlopen','dlclose','dlsym','XOpenDisplay','XCloseDisplay','XCreateColormap','XFreeColormap','XCreateWindow','XDestroyWindow','XAllocSizeHints','XGetVisualInfo']
        for level in ['O0','O2']:
            binary=out/level;run(level+'-build',[cc,*flags,'-'+level,*sources,*['-Wl,--wrap='+n for n in wraps],archive,runtime,
                *['-l'+n for n in meta['system_libraries']],'-Wl,-rpath,'+str(runtime.parent),'-o',binary])
            for mode in range(1,19):run(level+'-stage-'+str(mode),[binary,mode])
        r.update(complete=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()));r['passed']=r['inputsUnchanged'];assert r['passed']
    except Exception as error:r.update(complete=True,passed=False,error=str(error));raise
    finally:server.terminate();server.wait(timeout=10);log.close();save()


if __name__=='__main__':main()
