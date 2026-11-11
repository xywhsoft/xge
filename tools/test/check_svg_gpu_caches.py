"""Real SVG pixels and cached GPU lifetime in matched host/static products.

The additional aggregate form exercises CPU Font atlas admission and an owned
GlyphRun surviving its SVG. Linux uses headless software EGL; Windows runs one
bounded OpenGL frame per executable. No network or hardware-performance tests.
"""
from pathlib import Path
import argparse,base64,hashlib,json,os,shutil,struct,subprocess,zlib


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','test-root','engine','static','runtime','out'):
        p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();root,tests,engine,static,runtime,out=[v.resolve()for v in
        (a.root,a.test_root,a.engine,a.static,a.runtime,a.out)]
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    host=json.loads((engine/'build-report.json').read_text());archive=json.loads((static/'build-report.json').read_text())
    assert host['linked'] and archive['complete']and archive['passed']and archive['sourceUnchanged']
    assert Path(host['source_root']).resolve()==Path(archive['source_root']).resolve()==root
    assert host['features']==archive['features']and host['xrt_host_header_sha256']==archive['xrt_header_sha256']
    assert all(sha(v)==h for v,h in archive['inputs'].items())
    windows=os.name=='nt';enabled='XGE_ENABLE_EGL'in host['features'];assert enabled!=windows
    cc=Path(shutil.which('gcc')).resolve();image=engine/host['runtime_image'];library=engine/host['import_library']if windows else image
    source=tests/'tests/xge/xge_svg_gpu_cache_test.c';font=tests/'tests/xge/data/xge_ascii_context_fixture.ttf'
    files=[Path(__file__).resolve(),cc,source,font,image,library,runtime,engine/'build-report.json',static/'build-report.json',
        static/archive['archive'],engine/'xge_build_config.h',static/'xge_build_config.h']
    files.extend(Path(v)for v in archive['inputs']);files.extend(v for v in (engine/'host-xrt').iterdir()if v.is_file())
    inputs={str(v):sha(v)for v in files};out.mkdir(parents=True,exist_ok=False)
    def chunk(kind,data):
        return struct.pack('>I',len(data))+kind+data+struct.pack('>I',zlib.crc32(kind+data)&0xffffffff)
    png=b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',2,2,8,6,0,0,0))
    png+=chunk(b'IDAT',zlib.compress((b'\0'+bytes([255,0,0,255])*2)*2))+chunk(b'IEND',b'')
    document=out/'font-and-raster.svg'
    document.write_text('<svg width="16" height="16" viewBox="0 0 16 16"><style>'+
        "@font-face {font-family:'Local';src:url('data:font/ttf;base64,"+base64.b64encode(font.read_bytes()).decode()+"');}"+
        '</style><image x="0" y="0" width="8" height="8" href="data:image/png;base64,'+base64.b64encode(png).decode()+
        '"/><text font-family="Local" font-size="12" x="1" y="14" fill="white">abc</text></svg>')
    inputs[str(document)]=sha(document)
    env=dict(os.environ)
    if windows:
        for v in (runtime,image):shutil.copy2(v,out/v.name)
    else:
        env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+str(engine)+':'+env.get('LD_LIBRARY_PATH','')
        env.update(EGL_PLATFORM='surfaceless',LIBGL_ALWAYS_SOFTWARE='1');env.pop('DISPLAY',None);env.pop('WAYLAND_DISPLAY',None)
    report=dict(passed=False,shippingDelivery=False,scope=__doc__,actualGraphics=True,softwareEgl=enabled,
        hardwarePerformance=False,networkTests=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,command):
        command=list(map(str,command));result=subprocess.run(command,cwd=root,env=env,capture_output=True,timeout=240)
        log=out/(name+'.log');log.write_bytes(result.stdout+result.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=result.returncode,logSha256=sha(log)))
        save();print(name,result.returncode,flush=True);assert not result.returncode,log
    for variant,folder,lib in [('host',engine,library),('static',static,static/archive['archive']),('aggregate',static,static/archive['archive'])]:
        if variant=='aggregate':
            flags=[v for v in archive['flags']if v!='-O2'and not v.startswith('-DXGE_BUILD_ABI_FINGERPRINT=')]
            flags+=['-DXGE_SVG_TEST_AGGREGATE=1','-include',static/'xge_build_config.h']
        else:
            flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-DXGE_XRT_HOST=1','-include',folder/'xge_build_config.h']
            flags+=['-I'+str(v)for v in (root,root/'include',engine/'host-xrt',folder)]
            if variant=='static':flags+=['-DXGE_TEST_STATIC=1']
            elif windows:flags+=['-DXGE_DLL','-DXUI_DLL']
        systems=['-l'+v for v in archive['system_libraries']]if variant!='host'else[]
        if enabled:systems+=['-lEGL']
        if not windows:systems+=['-pthread','-ldl','-lm','-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine)]
        for level in ('O0','O2'):
            name=variant+'-'+level;binary=out/(name+('.exe'if windows else''));linkmap=out/(name+'.map')
            run(name+'-build',[cc,*flags,'-'+level,source,lib,runtime,*systems,'-Wl,--gc-sections','-Wl,-Map,'+str(linkmap),'-o',binary])
            if variant=='aggregate':
                member=next(Path(v['object']).name for v in archive['objects']if v['source'].replace('\\','/').endswith('single/xge.c'))
                assert '('+member+')'not in linkmap.read_text(errors='replace')
            run(name+'-run',[binary,document])
    report.update(passed=True,inputsUnchanged=all(sha(p)==h for p,h in inputs.items()));assert report['inputsUnchanged'];save()


if __name__=='__main__':main()
