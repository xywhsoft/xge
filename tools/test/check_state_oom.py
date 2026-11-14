"""Real Sokol/XGE pre-window allocation failures, cleanup and same-process retry.

Tests the vendored platform implementation and matched direct-XRT static archive.
Graphics recovery needs a local display (Linux may use Xvfb without TCP).
No remote services or network/security tests. Later backend allocations are out
of this gate's scope.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,time


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','test-root','static','runtime','out'):
        p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--graphics',action='store_true',help='Recover to a real local window after the failed runs')
    p.add_argument('--expect-old-abort',action='store_true',help='Reproduce the old assertion abort; not positive acceptance')
    a=p.parse_args();win=os.name=='nt'
    if a.graphics and not win and not os.environ.get('DISPLAY'):p.error('--graphics requires DISPLAY')
    root,tests,static,runtime,out=[v.resolve()for v in(a.root,a.test_root,a.static,a.runtime,a.out)]
    meta=json.loads((static/'build-report.json').read_text(encoding='utf-8'))
    assert meta['complete']and meta['passed']and meta['sourceUnchanged']and meta['xrt_mode']=='direct-external'
    assert Path(meta['source_root']).resolve()==root
    cc=Path(shutil.which('gcc')).resolve();archive=static/meta['archive']
    sources=[tests/'tests/xge/sokol_state_oom_test.c',tests/'tests/xge/xge_state_oom_test.c']
    sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest()
    files=[Path(__file__).resolve(),cc,*sources,archive,runtime,static/'build-report.json',
        static/'xge_build_config.h',static/'xge_static_xrt_contract.h',root/'extlibs/sokol/sokol_app.h',
        root/'src/xge/core/xge_impl.c',root/'src/xge/core/xge_core.c',root/'extlibs/xrt/xrt.h',
        *sorted((root/'include').glob('*.h'))]
    inputs={str(v):sha(v)for v in files};out.mkdir(parents=True,exist_ok=False)
    # Preserve the test/gate bytes even if the development tree changes later.
    for source in [Path(__file__).resolve(),*sources]:shutil.copy2(source,out/source.name)
    env=dict(os.environ);key='PATH'if win else'LD_LIBRARY_PATH'
    env[key]=str(runtime.parent)+os.pathsep+env.get(key,'')
    options={}
    if win:
        import ctypes
        ctypes.windll.kernel32.SetErrorMode(0x8003)
        options['creationflags']=subprocess.CREATE_NO_WINDOW
    else:
        import resource
        def no_core():resource.setrlimit(resource.RLIMIT_CORE,(0,0))
        options['preexec_fn']=no_core
    report=dict(complete=False,passed=False,scope=__doc__,shippingDelivery=False,networkTests=False,
        expectedOldAbort=a.expect_old_abort,graphics=a.graphics,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    def run(name,command,old=False,marker=None):
        command=list(map(str,command));start=time.monotonic()
        z=subprocess.run(command,cwd=out,env=env,capture_output=True,timeout=120,**options)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=z.returncode,expectedAbort=old,
            seconds=time.monotonic()-start,logSha256=sha(log)));save();print(name,z.returncode,flush=True)
        assert z.returncode==((3 if win else -6)if old else 0),log
        if old:assert b'ptr && (size > 0)'in log.read_bytes(),log
        if marker:assert marker in log.read_bytes(),log
    try:
        flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-parameter','-Wno-cast-function-type']
        systems=['-l'+v for v in meta['system_libraries']]
        for level in ('O0','O2'):
            for i,source in enumerate(sources):
                name=('sokol'if i==0 else'xge')+'-'+level;target=out/(name+('.exe'if win else''))
                if i==0:
                    args=[cc,*flags,'-'+level,'-I'+str(root),source,*systems,'-o',target]
                    if not a.expect_old_abort:args.insert(1,'-DXGE_TEST_STATE_OOM_FIXED=1')
                else:args=[cc,*flags,'-'+level,'-include',static/'xge_build_config.h',
                    *['-I'+str(v)for v in(root,root/'include',static)],source,archive,runtime,
                    *systems,*([]if win else['-Wl,-rpath,'+str(runtime.parent)]),'-o',target]
                run(name+'-build',args)
                marker=None if a.expect_old_abort else (b'Sokol state OOM:'if i==0 else b'XGE state OOM:')
                run(name+'-pre-window',[target],a.expect_old_abort,marker)
                if a.graphics:run(name+'-recover',[target,'recover'],a.expect_old_abort,marker)
        report.update(complete=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()))
        report['passed']=report['inputsUnchanged'];assert report['passed']
    except Exception as e:report.update(complete=True,passed=False,error=str(e));raise
    finally:save()


if __name__=='__main__':main()
