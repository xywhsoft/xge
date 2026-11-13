"""Exercise the pure legacy Image alpha operation in matched host/static SDKs.

Public C/C++ clients use stack pixel storage and no XRT or engine initializer.
The static link supplies no XRT/runtime or graphics/system library explicitly;
its reachable alpha code should need only the ordinary language CRT. This is
incremental library reachability evidence, not whole-engine performance proof.
"""
from pathlib import Path
import argparse,hashlib,json,os,re,shutil,subprocess


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','engine','static','out'):
        p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();root,host,static,out=[v.resolve()for v in(a.root,a.engine,a.static,a.out)]
    win=os.name=='nt';native=json.loads((static/'build-report.json').read_text());dynamic=json.loads((host/'build-report.json').read_text())
    assert native['complete']and native['passed']and native['sourceUnchanged']and dynamic['linked']
    assert Path(native['source_root']).resolve()==Path(dynamic['source_root']).resolve()==root
    image=host/dynamic['runtime_image'];library=host/dynamic['import_library']if win else image;archive=static/native['archive']
    tools={n:Path(shutil.which(n)).resolve()for n in ('gcc','g++','objdump')}
    case=root/'tests/xge/xge_image_alpha_test.c';sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest()
    files=[Path(__file__).resolve(),case,host/'build-report.json',static/'build-report.json',image,library,archive,*tools.values(),*map(Path,native['inputs'])]
    inputs={str(v):sha(v)for v in files};out.mkdir(parents=True,exist_ok=False)
    r=dict(complete=False,passed=False,shippingDelivery=False,scope=__doc__,inputs=inputs,rows=[],programs=[])
    env=os.environ.copy();key='PATH'if win else'LD_LIBRARY_PATH';env[key]=str(host)+os.pathsep+env.get(key,'')
    def save():(out/'results.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf-8')
    def run(name,command):
        command=list(map(str,command));z=subprocess.run(command,cwd=out,env=env,capture_output=True,timeout=180)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        r['rows'].append(dict(name=name,command=command,exitCode=z.returncode,logSha256=sha(log)));save();print(name,z.returncode,flush=True);assert not z.returncode,log
        return z.stdout+z.stderr
    flags=[v for v in native['flags']if v not in ('-O2','-std=c11','-DXGE_STATIC_BUILD=1')and not v.startswith('-DXGE_BUILD_ABI_FINGERPRINT=')]
    flags+=['-DXGE_TEST_IMAGE_ALPHA_CHECKED=1']
    flags=['-I'+str(root/v[2:])if v.startswith('-I')and not Path(v[2:]).is_absolute()else v for v in flags]
    try:
        for kind,folder,link in [('host',host,library),('static',static,archive)]:
            defines=['-DXGE_DLL=1','-DXUI_DLL=1']if kind=='host'else['-DXGE_STATIC_BUILD=1']
            for standard,level in [('c11','O0'),('c11','O2'),('c++11','O2'),('c++17','O2')]:
                name=kind+'-'+standard.replace('+','p')+'-'+level;binary=out/(name+('.exe'if win else''));cc=tools['g++']if '+'in standard else tools['gcc']
                run(name+'-build',[cc,*flags,'-std='+standard,'-'+level,'-Werror',*defines,'-I'+str(folder),'-include',folder/'xge_build_config.h',case,link,'-Wl,--gc-sections',*([]if win else['-Wl,-rpath,'+str(host)]),'-o',binary])
                assert b'rounding and legacy repeat passed'in run(name+'-run',[binary])
            table=run(kind+'-imports',[tools['objdump'],'-p',binary]);imports=re.findall(rb'DLL Name:\s*(\S+)',table)if win else re.findall(rb'\bNEEDED\s+(\S+)',table)
            assert not any(b'xrt'in v.lower()or v.lower()in{b'xl.dll',b'xlc.dll',b'libxl.so',b'libxlc.so'}for v in imports)
            assert any(b'xge3_host'in v.lower()for v in imports)==(kind=='host')
            r['programs'].append(dict(path=str(binary),sha256=sha(binary),bytes=binary.stat().st_size,imports=[v.decode()for v in imports]));save()
        r.update(complete=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()));r['passed']=r['inputsUnchanged'];assert r['passed']
    except Exception as e:r.update(complete=True,passed=False,error=str(e));raise
    finally:save()


if __name__=='__main__':main()
