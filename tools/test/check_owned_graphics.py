"""Native owned graphics CPU gate against one matched, explicit host runtime.

Does not certify a graphics device, an xlang compiler or a shipping delivery.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,time


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','engine','runtime','host-header','out'):p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--test-root',type=Path,help='Separate test sources; --root remains the exact immutable engine SDK')
    p.add_argument('--graphics',action='store_true',help='Also run the bounded real OpenGL window test')
    a=p.parse_args();root,engine,runtime,header,out=[x.resolve() for x in (a.root,a.engine,a.runtime,a.host_header,a.out)]
    sha=lambda path:hashlib.sha256(Path(path).read_bytes()).hexdigest()
    meta=json.loads((engine/'build-report.json').read_text())
    assert meta['linked'] and meta['xrt_mode']=='host'
    assert meta['xrt_host_header_sha256']==hashlib.sha256(header.read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    assert meta['xrt_host_header_sha256']==hashlib.sha256((root/'extlibs/xrt/xrt.h').read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    tests=a.test_root.resolve() if a.test_root else root
    cc=shutil.which('gcc');assert cc
    native=engine/meta['runtime_image'];library=engine/meta['import_library'] if os.name=='nt' else native
    font=tests/'tests/xge/data/xge_opentype_fixture.ttf'
    cases=[('resource-providers',tests/'tests/xge/xge_resource_owned_test.c',[]),
           ('owned-graphics',tests/'tests/xge/xge_owned_graphics_test.c',[]),
           ('storage',tests/'tests/xge/xge_storage_adoption_test.c',[font]),
           ('proxy',tests/'tests/xui/xui_proxy_xge_owned_test.c',[font]),
           ('global-callbacks',tests/'tests/xge/xge_owned_global_test.c',[])]
    if a.graphics:cases.append(('graphics-device',tests/'tests/xge/xge_owned_graphics_gpu_test.c',[]))
    pins={Path(__file__).resolve(),Path(cc),header,native,library,runtime,font,engine/'build-report.json',
          engine/'xge_build_config.h',root/'include/xge.h',root/'include/xui.h',root/'extlibs/xrt/xrt.h'}
    pins.update(source for _,source,_ in cases)
    pins.update(x for x in (engine/'host-xrt').iterdir() if x.is_file())
    inputs={str(x):sha(x) for x in pins};out.mkdir(parents=True,exist_ok=False);(out/'tmp').mkdir()
    env=dict(os.environ,TEMP=str(out/'tmp'),TMP=str(out/'tmp'),TMPDIR=str(out/'tmp'))
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name);shutil.copy2(native,out/native.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+str(engine)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(passed=False,scope=__doc__,graphicsDevice=a.graphics,networkTests=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    def run(name,command):
        command=list(map(str,command));start=time.monotonic()
        z=subprocess.run(command,cwd=root,env=env,capture_output=True,timeout=240)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=z.returncode,seconds=round(time.monotonic()-start,3),logSha256=sha(log)))
        save();print(name,z.returncode,flush=True);assert not z.returncode,log
    flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-DXGE_XRT_HOST=1','-include',engine/'xge_build_config.h']
    flags+=['-I'+str(x) for x in (root,root/'include',engine/'host-xrt')]
    if os.name=='nt':flags+=['-DXGE_DLL','-DXUI_DLL']
    systems=[] if os.name=='nt' else ['-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine),'-pthread','-ldl','-lm']
    for name,source,arguments in cases:
        for opt in ('O0','O2'):
            exe=out/(name+'-'+opt+('.exe' if os.name=='nt' else ''))
            run(name+'-'+opt+'-build',[cc,*flags,'-'+opt,source,library,runtime,*systems,'-o',exe])
            run(name+'-'+opt+'-run',[exe,*arguments])
    assert all(sha(path)==digest for path,digest in inputs.items())
    report.update(passed=True,inputsUnchanged=True);save()


if __name__=='__main__':main()
