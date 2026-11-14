"""Actual direct-XRT static XGE resolver ownership, checked close retry and restart.

One full runtime; ELF/PE platform-call wrapping, no replacement engine TU.
Local windows (Linux Xvfb has TCP disabled); no network/security tests.
"""
from pathlib import Path
import argparse,hashlib,json,os,select,shutil,subprocess,time


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ['root','test-root','static','runtime','out']:p.add_argument('--'+n,type=Path,required=True)
    p.add_argument('--legacy-static',type=Path)
    a=p.parse_args();win=os.name=='nt';root,tests,static,runtime,out=[v.resolve()for v in(a.root,a.test_root,a.static,a.runtime,a.out)]
    meta=json.loads((static/'build-report.json').read_text(encoding='utf-8'));assert meta['passed']and meta['sourceUnchanged']and meta['xrt_mode']=='direct-external'
    cc=Path(shutil.which('gcc')).resolve();archive=static/meta['archive']
    xvfb=None if win else Path(shutil.which('Xvfb')).resolve()
    sources=[tests/'tests/xge/xge_gl_library_engine_test.c'];sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest()
    files=[Path(__file__).resolve(),*sources,cc,archive,runtime,static/'build-report.json',
        static/'xge_build_config.h',static/'xge_static_xrt_contract.h',root/'extlibs/sokol/sokol_app.h',root/'extlibs/xrt/xrt.h',*sorted((root/'include').glob('*.h')),*[root/n for n in ('src/xge/core/xge_impl.c','src/xge/core/xge_core.c','src/xge/core/xge_command.c','src/xge/render/xge_gl_library.h')]]
    if xvfb:files.append(xvfb)
    if a.legacy_static:
        old=a.legacy_static.resolve();oldmeta=json.loads((old/'build-report.json').read_text(encoding='utf-8'));assert oldmeta['passed'] and oldmeta['sourceUnchanged']
        oldarchive=old/oldmeta['archive'];files.extend([old/'build-report.json',oldarchive])
    inputs={str(v):sha(v)for v in files};out.mkdir(parents=True,exist_ok=False);snapshots=out/'input-snapshots';snapshots.mkdir()
    for f in [Path(__file__).resolve(),*sources]:shutil.copy2(f,snapshots/f.name)
    r=dict(complete=False,passed=False,scope=__doc__,shippingDelivery=False,networkTests=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf-8')
    env=dict(os.environ,LIBGL_ALWAYS_SOFTWARE='1');server=None;log=None
    env['PATH'if win else'LD_LIBRARY_PATH']=str(runtime.parent)+os.pathsep+env.get('PATH'if win else'LD_LIBRARY_PATH','')
    def no_core():
        import resource
        resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    try:
        if not win:
            log=(out/'xvfb.log').open('wb');server=subprocess.Popen([xvfb,'-displayfd','1','-screen','0','640x480x24','-nolisten','tcp','-noreset'],stdout=subprocess.PIPE,stderr=log)
            assert select.select([server.stdout],[],[],15)[0];number=server.stdout.readline().decode().strip();assert number.isdigit();env['DISPLAY']=':'+number
        def run(name,command,refused=False):
            command=list(map(str,command));t=time.monotonic();z=subprocess.run(command,cwd=out,env=env,capture_output=True,timeout=90,preexec_fn=None if win else no_core)
            f=out/(name+'.log');f.write_bytes(z.stdout+z.stderr);r['rows'].append(dict(name=name,command=command,exitCode=z.returncode,seconds=time.monotonic()-t,logSha256=sha(f),refused=refused));save();print(name,z.returncode,flush=True);assert (z.returncode!=0)==refused,f;return z.stdout+z.stderr
        flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-include',static/'xge_build_config.h',*['-I'+str(v)for v in(root,root/'include',static)]]
        wraps=['__imp_LoadLibraryA','__imp_LoadLibraryExA','__imp_FreeLibrary','__imp_GetProcAddress']if win else['dlopen','dlclose','dlsym']
        rpath=[]if win else['-Wl,-rpath,'+str(runtime.parent)]
        for level in ['O0','O2']:
            binary=out/(level+'.exe'if win else level);run(level+'-build',[cc,*flags,'-'+level,*sources,*['-Wl,--wrap='+n for n in wraps],archive,runtime,
                *['-l'+n for n in meta['system_libraries']],*rpath,'-o',binary])
            for mode in range(4):run(level+'-mode-'+str(mode),[binary,mode])
            if a.legacy_static:
                oldbinary=out/(level+'-legacy'+('.exe'if win else''));run(level+'-legacy-build',[cc,*flags,'-'+level,*sources,*['-Wl,--wrap='+n for n in wraps],oldarchive,runtime,*['-l'+n for n in oldmeta['system_libraries']],*rpath,'-o',oldbinary])
                result=run(level+'-legacy-cache-reproduced',[oldbinary,3],True)
                assert b'After Unit: resolver libraries=1' in result
        r.update(complete=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()));r['passed']=r['inputsUnchanged'];assert r['passed']
    except Exception as error:r.update(complete=True,passed=False,error=str(error));raise
    finally:
        if server:server.terminate();server.wait(timeout=10)
        if log:log.close()
        save()


if __name__=='__main__':main()
