"""Qualify actual hosted/static SDK 3D Scene lifetime; CPU only, no shipping.

No production replacement objects or GL stubs enter the test programs. Input
hashes pin the exact SDK, compiler runtime and separately selected test source.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,time


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','test-root','engine','static-build','runtime','out'):
        p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();root,tests,engine,static,runtime,out=[v.resolve() for v in
        (a.root,a.test_root,a.engine,a.static_build,a.runtime,a.out)]
    sha=lambda path:hashlib.sha256(Path(path).read_bytes()).hexdigest()
    host=json.loads((engine/'build-report.json').read_text(encoding='utf8'))
    stat=json.loads((static/'build-report.json').read_text(encoding='utf8'))
    assert host['linked'] and host['xrt_mode']=='host'
    assert stat['passed'] and stat['sourceUnchanged'] and stat['xrt_mode']=='direct-external'
    image=engine/host['runtime_image'];library=engine/host['import_library'] if os.name=='nt' else image
    archive=static/stat['archive'];cc=Path(shutil.which('gcc')).resolve()
    fixture=tests/'tests/xge/xge3d_scene_owned_test.c'
    files=[Path(__file__).resolve(),cc,fixture,root/'include/xge.h',root/'src/xge/3d/xge3d_internal.h',
        root/'src/xge/3d/xge3d_scene.c',root/'src/xge/core/xge_data_lifetime.inl',
        root/'extlibs/xrt/xrt.h',image,library,archive,runtime,
        engine/'build-report.json',static/'build-report.json',engine/'xge_build_config.h',static/'xge_build_config.h']
    files.extend(v for v in (engine/'host-xrt').iterdir() if v.is_file())
    inputs={str(v):sha(v) for v in files};out.mkdir(parents=True,exist_ok=False)
    report=dict(complete=False,passed=False,scope=__doc__,inputs=inputs,rows=[],graphicsTests=False,
        networkTests=False,productionObjectOverrides=False,shippingDelivery=False)
    def save():
        tmp=out/'results.json.tmp';tmp.write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');os.replace(tmp,out/'results.json')
    env=dict(os.environ,TMP=str(out),TEMP=str(out),TMPDIR=str(out))
    if os.name=='nt':
        for item in (image,runtime):shutil.copy2(item,out/item.name)
    else:env['LD_LIBRARY_PATH']=str(engine)+':'+str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    def run(name,command,marker=None):
        command=list(map(str,command));start=time.monotonic()
        z=subprocess.run(command,cwd=root,env=env,capture_output=True,timeout=180)
        data=z.stdout+z.stderr;log=out/(name+'.log');log.write_bytes(data)
        report['rows'].append(dict(name=name,command=command,exitCode=z.returncode,seconds=time.monotonic()-start,logSha256=sha(log)))
        save();print(name,z.returncode,flush=True);assert z.returncode==0,log
        if marker:assert marker in data,log
    try:
        for name,folder,link in (('host',engine,library),('static',static,archive)):
            flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-DXGE_XRT_HOST=1',
                '-include',folder/'xge_build_config.h','-I'+str(folder),'-I'+str(engine/'host-xrt'),
                '-I'+str(root),'-I'+str(root/'include')]
            if os.name=='nt' and name=='host':flags+=['-DXGE_DLL=1','-DXUI_DLL=1']
            if name=='static':flags+=['-DXGE_TEST_STATIC=1']
            systems=['-l'+v for v in stat['system_libraries']] if name=='static' else []
            if os.name!='nt':systems+=['-ldl','-lm','-pthread']
            for opt in ('O0','O2'):
                stem=name+'-'+opt;binary=out/(stem+('.exe' if os.name=='nt' else ''))
                run(stem+'-build',[cc,*flags,'-'+opt,fixture,link,runtime,*systems,'-o',binary])
                run(stem+'-run',[binary],b'XGE3D_SCENE_OWNED_NATIVE_OK')
        for name,defines in (('full',['-DXGE_ENABLE_3D=1']),('core-only',['-DXGE_ENABLE_3D=1','-DXGE3D_ENABLE_SCENE=0'])):
            run('syntax-'+name,[cc,'-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-unused-function',
                '-Wno-cast-function-type',*defines,'-I'+str(root),'-I'+str(root/'include'),'-I'+str(root/'src/xge/core'),
                '-fsyntax-only',root/'src/xge/3d/xge3d_scene.c'])
        # Disabled builds omit the 3D implementation units entirely. Their
        # private header is intentionally not a public disabled-feature header.
        run('syntax-disabled-public-header',[cc,'-std=c11','-Wall','-Wextra','-Werror',
            '-DXGE_ENABLE_3D=0','-I'+str(root),'-I'+str(root/'include'),'-x','c','-fsyntax-only',root/'include/xge.h'])
        report.update(complete=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()))
        report['passed']=report['inputsUnchanged'];assert report['passed']
    except BaseException as error:report.update(complete=True,error=repr(error));raise
    finally:save()


if __name__=='__main__':main()
