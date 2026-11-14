"""Calibrated constructor and menu-copy allocation failures.
O0/O2 hosted/static clients, 56 default Widget constructors, five nonempty
descriptors and six menu mutation cases. Full Context retirement distinguishes
retained queue capacities from unreachable allocations. No language, GPU,
network or shipping-delivery certification.
"""
from pathlib import Path
import argparse,hashlib,json,os,re,shutil,subprocess,time


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in('root','freeze','engine','static','runtime','out'):
        p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();root,freeze,engine,static,runtime,out=[v.resolve()for v in
        (a.root,a.freeze,a.engine,a.static,a.runtime,a.out)]
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();win=os.name=='nt'
    frozen=json.loads(freeze.read_text(encoding='utf-8'))
    assert all(sha(root/n)==h for n,h in frozen['files'].items())
    h=json.loads((engine/'build-report.json').read_text(encoding='utf-8'))
    s=json.loads((static/'build-report.json').read_text(encoding='utf-8'))
    assert h['linked']and h['xrt_mode']=='host'and s['passed']and s['sourceUnchanged']
    image=engine/h['runtime_image'];library=engine/h['import_library']if win else image
    archive=static/s['archive'];cc=Path(shutil.which('gcc')).resolve()
    constructors=root/'tests/xui/xui_constructor_retirement_test.c'
    menu=root/'tests/xui/xui_menu_copy_failure_test.c'
    declared=re.findall(r'XUI_API int (xui\w+Create)\(xui_context \w+, xui_widget\* \w+, const \w+\* \w+\);',(root/'include/xui.h').read_text(encoding='utf-8'))
    tested=re.findall(r'\{"(xui\w+Create)",create\d+\}',constructors.read_text(encoding='utf-8'))
    assert len(declared)==len(tested)==56 and set(declared)==set(tested)
    files=[Path(__file__),cc,freeze,constructors,menu,image,library,archive,runtime,
        engine/'build-report.json',static/'build-report.json',engine/'xge_build_config.h',
        static/'xge_build_config.h',static/'xge_static_xrt_contract.h',
        *sorted((engine/'host-xrt').glob('*')),*sorted((root/'include').glob('*.h'))]
    inputs={str(v):sha(v)for v in files};out.mkdir(parents=True,exist_ok=False)
    for v in(constructors,menu):shutil.copy2(v,out/v.name)
    r=dict(complete=False,passed=False,shippingDelivery=False,scope=__doc__,networkTests=False,
        languageBindingsQualified=False,inputs=inputs,rows=[],coverage=tested,calibration=[])
    def save():(out/'results.json').write_bytes((json.dumps(r,indent=2)+'\n').encode())
    env=dict(os.environ);key='PATH'if win else'LD_LIBRARY_PATH'
    env[key]=os.pathsep.join((str(engine),str(runtime.parent),env.get(key,'')))
    def no_core():
        import resource
        resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    def run(name,cmd):
        cmd=list(map(str,cmd));start=time.monotonic()
        z=subprocess.run(cmd,cwd=root,env=env,capture_output=True,timeout=180,
            preexec_fn=None if win else no_core)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        r['rows'].append(dict(name=name,command=cmd,exitCode=z.returncode,
            seconds=time.monotonic()-start,logSha256=sha(log)))
        save();print(name,z.returncode,flush=True);assert not z.returncode,log
        return z.stdout+z.stderr
    try:
        for kind,folder,lib in(('host',engine,library),('static',static,archive)):
            flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type',
                '-Wno-unused-function','-DXGE_XRT_HOST=1','-include',folder/'xge_build_config.h',
                *['-I'+str(v)for v in(root,root/'include',folder,engine/'host-xrt')]]
            flags+=['-DXGE_TEST_STATIC=1']if kind=='static'else['-DXGE_DLL=1','-DXUI_DLL=1']if win else[]
            systems=['-l'+v for v in s['system_libraries']]if kind=='static'else[]
            if not win:systems+=['-Wl,-rpath,'+str(engine),'-Wl,-rpath,'+str(runtime.parent),'-ldl','-lm','-pthread']
            for opt in('O0','O2'):
                for label,case,count in(('constructors',constructors,61),('menu',menu,6)):
                    name=kind+'-'+opt+'-'+label;binary=out/(name+('.exe'if win else''))
                    run(name+'-build',[cc,*flags,'-'+opt,case,lib,runtime,*systems,'-o',binary])
                    for i in range(count):
                        data=run(name+'-'+str(i),[binary,str(i)])
                        assert b'retirement passed'in data
                        m=re.search(rb'prefixes=(\d+)',data);assert m and int(m.group(1))>=8
                        r['calibration'].append(dict(kind=kind,opt=opt,family=label,index=i,prefixes=int(m.group(1))))
                        save()
        assert len(r['rows'])==276 and len(r['calibration'])==268
        r.update(complete=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()),
            frozenSourcesUnchanged=all(sha(root/n)==h for n,h in frozen['files'].items()))
        r['passed']=r['inputsUnchanged']and r['frozenSourcesUnchanged'];assert r['passed']
    except Exception as e:r.update(complete=True,passed=False,error=str(e));raise
    finally:save()


if __name__=='__main__':main()
