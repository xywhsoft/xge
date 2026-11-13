"""Actual X11 connection-failure unwind, checked XGE result and same-process retry.

The standalone vendored Sokol and actual direct-XRT static XGE are both tested.
Graphics mode needs a local X server; use Xvfb with TCP listening disabled for
software-GL acceptance. No attack, remote service or network protocol tests.
"""
from pathlib import Path
import argparse,hashlib,json,os,resource,shutil,signal,subprocess,time


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','test-root','static','runtime','out'):p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--graphics',action='store_true',help='Also recover to a real local display in the same process')
    p.add_argument('--expect-rejection',action='store_true',help='Reproduce the old native SIGSEGV; not positive acceptance')
    a=p.parse_args();assert os.name!='nt','Actual Linux X11 gate only'
    if a.graphics and not os.environ.get('DISPLAY'):p.error('--graphics needs DISPLAY (e.g. xvfb-run)')
    root,tests,static,runtime,out=[v.resolve()for v in(a.root,a.test_root,a.static,a.runtime,a.out)]
    meta=json.loads((static/'build-report.json').read_text());assert meta['passed']and meta['sourceUnchanged']and meta['xrt_mode']=='direct-external'
    cc=Path(shutil.which('gcc')).resolve();archive=static/meta['archive']
    sources=[tests/'tests/xge/sokol_x11_startup_lifetime_test.c',tests/'tests/xge/xge_x11_startup_test.c']
    sha=lambda x:hashlib.sha256(Path(x).read_bytes()).hexdigest()
    files=[Path(__file__),cc,*sources,archive,runtime,static/'build-report.json',static/'xge_build_config.h',
        static/'xge_static_xrt_contract.h',root/'extlibs/sokol/sokol_app.h',root/'src/xge/core/xge_impl.c',
        root/'src/xge/core/xge_core.c',root/'extlibs/xrt/xrt.h',*sorted((root/'include').glob('*.h'))]
    inputs={str(f):sha(f)for f in files};out.mkdir(parents=True,exist_ok=False)
    report=dict(complete=False,passed=False,scope=__doc__,shippingDelivery=False,networkTests=False,
        expectedOldSigsegv=a.expect_rejection,graphics=a.graphics,inputs=inputs,rows=[])
    env=dict(os.environ);env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    def no_core():resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    def run(name,command,old=False):
        command=list(map(str,command));start=time.monotonic()
        z=subprocess.run(command,cwd=out,env=env,capture_output=True,timeout=90,preexec_fn=no_core)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=z.returncode,expectedSigsegv=old,
            seconds=time.monotonic()-start,logSha256=sha(log)))
        save();print(name,z.returncode,flush=True)
        assert z.returncode==(-signal.SIGSEGV if old else 0),log
    try:
        flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-parameter','-Wno-cast-function-type']
        systems=['-l'+v for v in meta['system_libraries']]
        for level in ('O0','O2'):
            for i,source in enumerate(sources):
                name=('sokol'if i==0 else'xge')+'-'+level;target=out/name
                if i==0:args=[cc,*flags,'-'+level,'-I'+str(root),source,*systems,'-o',target]
                else:args=[cc,*flags,'-'+level,'-include',static/'xge_build_config.h',
                    *['-I'+str(v)for v in(root,root/'include',static)],source,archive,runtime,
                    *systems,'-Wl,-rpath,'+str(runtime.parent),'-o',target]
                run(name+'-build',args);run(name+'-no-display',[target],a.expect_rejection)
                if a.graphics:run(name+'-recover',[target,'recover'],a.expect_rejection)
        report.update(complete=True,inputsUnchanged=all(sha(f)==h for f,h in inputs.items()))
        report['passed']=report['inputsUnchanged'];assert report['passed']
    except Exception as error:report.update(complete=True,passed=False,error=str(error));raise
    finally:save()


if __name__=='__main__':main()
