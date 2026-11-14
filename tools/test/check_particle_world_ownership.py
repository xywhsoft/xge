"""Exact managed ParticleWorld native ownership, shared O2 SDK and sole full XRT.

Eight independent GCC O0/O2 hosted/static clients reuse 10237 kernel assertions,
explicit managed/raw contracts, graph/active-release/OOM/thread/cold/hot checks.
No language ParticleWorld, persistent owned collision, GPU or network claim.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in('root','engine','static','runtime','out'):p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--test-root',type=Path,help='Explicit later fixture overlay; native product stays at --root')
    p.add_argument('--legacy-engine',type=Path,help='Prior hosted candidate, exact constructor snapshot rejection only')
    a=p.parse_args();root,engine,static,runtime,out=[v.resolve()for v in(a.root,a.engine,a.static,a.runtime,a.out)]
    win=os.name=='nt';cc=Path(shutil.which('gcc')).resolve();sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest()
    h=json.loads((engine/'build-report.json').read_text(encoding='utf-8'));s=json.loads((static/'build-report.json').read_text(encoding='utf-8'))
    assert h['linked']and h['xrt_mode']=='host'and s['passed']and s['sourceUnchanged']and s['xrt_mode']=='direct-external'
    image=engine/h['runtime_image'];library=engine/h['import_library']if win else image;archive=static/s['archive']
    tests=a.test_root.resolve()if a.test_root else root
    case=tests/'tests/xge/xge_particle_world_owned_test.c'
    files=[Path(__file__).resolve(),cc,case,tests/'tests/xge/test_particle.c',image,library,archive,runtime,
        engine/'build-report.json',static/'build-report.json',engine/'xge_build_config.h',static/'xge_build_config.h',static/'xge_static_xrt_contract.h',
        *sorted((root/'include').glob('*.h')),*sorted((engine/'host-xrt').glob('*'))]
    files.extend(root/'src/xge/particle'/v for v in('xge_particle.c','xge_particle_owned.inl','xge_particle_io.c','xge_particle_internal.h'))
    files.extend(root/'src/xge/core'/v for v in('xge_data_lifetime.inl','xge_data_lifetime_internal.h'))
    inputs={str(v):sha(v)for v in files};out.mkdir(parents=True,exist_ok=False)
    if a.legacy_engine:
        legacy=a.legacy_engine.resolve();old=json.loads((legacy/'build-report.json').read_text(encoding='utf-8'))
        old_library=legacy/old['import_library']if win else legacy/old['runtime_image']
        inputs.update({str(v):sha(v)for v in(legacy/'build-report.json',legacy/old['runtime_image'],old_library)})
    report=dict(complete=False,passed=False,scope=__doc__,shippingDelivery=False,networkTests=False,
        languageWorldQualified=False,persistentOwnedCollisionQualified=False,fixtureRoot=str(tests),nativeRoot=str(root),inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    env=dict(os.environ);key='PATH'if win else'LD_LIBRARY_PATH';env[key]=os.pathsep.join((str(engine),str(runtime.parent),env.get(key,'')))
    def run(name,command,refused=False,override=None):
        current=dict(env)
        if override:current[key]=str(override)+os.pathsep+current[key]
        def no_core():
            import resource
            resource.setrlimit(resource.RLIMIT_CORE,(0,0))
        command=list(map(str,command));z=subprocess.run(command,env=current,cwd=out,capture_output=True,timeout=180,preexec_fn=None if win else no_core)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=z.returncode,refused=refused,logSha256=sha(log)));save();print(name,z.returncode,flush=True)
        assert (z.returncode!=0)==refused,log;return z.stdout+z.stderr
    try:
        for kind,folder,lib in(('host',engine,library),('static',static,archive)):
            flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-DXGE_XRT_HOST=1','-DXRT_MODULE_MEMORY_DEBUG',
                '-include',folder/'xge_build_config.h',*['-I'+str(v)for v in(root,root/'include',folder,engine/'host-xrt')]]
            flags+=['-DPARTICLE_SDK_STATIC=1']if kind=='static'else['-DPARTICLE_SDK_HOST=1']
            if win:flags+=['-DXRT_USE_SHARED=1',*(['-DXGE_DLL=1','-DXUI_DLL=1']if kind=='host'else[])]
            systems=['-l'+v for v in s['system_libraries']]if kind=='static'else[]
            if not win:systems+=['-Wl,-rpath,'+str(engine),'-Wl,-rpath,'+str(runtime.parent),'-ldl','-lm','-pthread']
            for level in('O0','O2'):
                for profile in('raw','managed'):
                    name=kind+'-'+level+'-'+profile;binary=out/(name+('.exe'if win else''))
                    options=['-DPARTICLE_RAW_KERNEL=1']if profile=='raw'else[]
                    run(name+'-build',[cc,*flags,*options,'-'+level,case,lib,runtime,*systems,'-o',binary])
                    output=run(name,[binary]);assert b'PASS particle runtime: 10237 checks'in output
                    assert b'ownership: native shared refs, active release, graph, checked failures and thread admission passed'in output
                    if kind=='host'and level=='O2'and profile=='managed'and a.legacy_engine:
                        old_binary=out/('legacy'+('.exe'if win else''))
                        run('legacy-build',[cc,*flags,'-O2',case,old_library,runtime,*systems,'-o',old_binary])
                        output=run('legacy-snapshot-rejected',[old_binary,'--snapshot-only'],True,legacy)
                        assert b'Owned constructor input snapshot'in output
        report.update(complete=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()));report['passed']=report['inputsUnchanged'];assert report['passed']
    except Exception as error:report.update(complete=True,passed=False,error=str(error));raise
    finally:save()


if __name__=='__main__':main()
