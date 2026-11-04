"""Matched default XGE proxy/resource backend CPU acceptance.

Only CPU RGBA pack/read rows, font decoding, metrics, retained glyph shaping,
native RC and checked cleanup are executed. No engine/window/device/GPU/network is initialized.
This does not certify Context/Widget cache graphs or the language package.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,time
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','engine','runtime','host-header','font','out'):
        p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();root,engine,runtime,header,font,out=[v.resolve() for v in (a.root,a.engine,a.runtime,a.host_header,a.font,a.out)]
    metadata=json.loads((engine/'build-report.json').read_text());assert metadata['linked'] and metadata['xrt_mode']=='host'
    sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest()
    assert metadata['xrt_host_header_sha256']==hashlib.sha256(header.read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    cc=shutil.which('gcc');assert cc
    binary=engine/('xge.dll' if os.name=='nt' else 'libxge.so');library=engine/('xge.lib' if os.name=='nt' else 'libxge.so')
    source=root/'tests/xui/xui_proxy_xge_owned_test.c'
    files=[Path(__file__).resolve(),Path(cc),source,runtime,header,font,binary,library,engine/'build-report.json',engine/'xge_build_config.h',
        *[root/name for name in ('src/xui/core/xui_proxy_resource.c','src/xui/core/xui_proxy_resource.h','src/xui/core/xui_proxy_xge.c','src/xui/core/xui_proxy_xge_shape_paint.inl','src/xui/core/xui_text.c','src/xge/text/xge_texture.c','src/xge/render/xge_render_target.c','extlibs/xrt/xrt.h','extlibs/xrt/xrt_config.h','config/modules.json')],
        *[v for v in (engine/'host-xrt').iterdir() if v.is_file()],*(root/'include').glob('*.h')]
    inputs={str(v):sha(v) for v in files};out.mkdir(parents=True,exist_ok=False)
    env=dict(os.environ)
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name);shutil.copy2(binary,out/binary.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+str(engine)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(complete=False,passed=False,scope=__doc__,inputs=inputs,rows=[],gpuTests=False,networkTests=False,fullLanguagePackageAcceptance=False)
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    for opt in ('O0','O2'):
        program=out/(opt+('.exe' if os.name=='nt' else ''))
        flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-DXGE_XRT_HOST=1','-include',engine/'xge_build_config.h',*['-I'+str(v) for v in (root,root/'include',engine/'host-xrt')]]
        if os.name=='nt':flags+=['-DXGE_DLL','-DXUI_DLL']
        links=[] if os.name=='nt' else ['-pthread','-ldl','-lm','-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine)]
        for name,command in ((opt+'-build',[cc,*flags,'-'+opt,source,library,runtime,*links,'-o',program]),(opt+'-run',[program,font])):
            start=time.monotonic();r=subprocess.run(list(map(str,command)),cwd=root,env=env,capture_output=True,timeout=180)
            log=out/(name+'.log');log.write_bytes(r.stdout+r.stderr)
            report['rows'].append(dict(name=name,command=list(map(str,command)),exitCode=r.returncode,seconds=round(time.monotonic()-start,3),logSha256=sha(log)));save()
            print(name,r.returncode,flush=True);assert not r.returncode,log
    report.update(complete=True,passed=all(sha(v)==value for v,value in inputs.items()));save();assert report['passed']
if __name__=='__main__':main()
