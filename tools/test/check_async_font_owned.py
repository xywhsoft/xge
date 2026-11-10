"""Matched owned Font requests: caller-thread completion, move and failure cleanup.

Generated valid font only; no window, GPU, audio device or network tests.
Host and direct-XRT static inputs must use the same complete public SDK.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('root','test-root','engine','static','runtime','host-header','out'):
  p.add_argument('--'+name,type=Path,required=True)
 a=p.parse_args();root,tests,engine,static,runtime,header,out=[v.resolve() for v in
  (a.root,a.test_root,a.engine,a.static,a.runtime,a.host_header,a.out)]
 sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest()
 canonical=lambda v:hashlib.sha256(Path(v).read_bytes().replace(b'\r\n',b'\n')).hexdigest()
 host=json.loads((engine/'build-report.json').read_text());archive=json.loads((static/'build-report.json').read_text())
 assert host['linked'] and host['xrt_mode']=='host' and archive['passed'] and archive['sourceUnchanged']
 assert host['features']==archive['features'] and host['xrt_host_header_sha256']==archive['xrt_header_sha256']==canonical(header)==canonical(root/'extlibs/xrt/xrt.h')
 win=os.name=='nt';cc=Path(shutil.which('gcc')).resolve();image=engine/host['runtime_image'];lib=engine/host['import_library'] if win else image
 source=tests/'tests/xge/xge_async_font_owned_test.c';font=tests/'tests/xge/data/xge_opentype_fixture.ttf'
 files=[Path(__file__).resolve(),source,font,cc,runtime,header,image,lib,engine/'build-report.json',static/'build-report.json',static/archive['archive'],static/'xge_build_config.h',static/'xge_static_xrt_contract.h',engine/'xge_build_config.h',root/'src/xge/resource/xge_async_owned.c',root/'src/xge/resource/xge_async.c',root/'src/xge/text/xge_font.c']
 files.extend((root/'include').glob('*.h'));files.extend(v for v in (engine/'host-xrt').iterdir() if v.is_file())
 inputs={str(v):sha(v) for v in files};out.mkdir(parents=True,exist_ok=False);env=dict(os.environ)
 if win:
  for v in (image,runtime):shutil.copy2(v,out/v.name)
 else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+str(engine)+':'+env.get('LD_LIBRARY_PATH','')
 report=dict(passed=False,scope=__doc__,shippingDelivery=False,graphicsDevice=False,networkTests=False,inputs=inputs,rows=[])
 def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
 def run(name,args):
  args=list(map(str,args));r=subprocess.run(args,cwd=out,env=env,capture_output=True,timeout=90)
  log=out/(name+'.log');log.write_bytes(r.stdout+r.stderr)
  report['rows'].append(dict(name=name,command=args,exitCode=r.returncode,logSha256=sha(log)))
  save();print(name,r.returncode,flush=True)
  return r.returncode
 for kind,folder,library in [('host',engine,lib),('static',static,static/archive['archive'])]:
  flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-DXGE_XRT_HOST=1','-include',folder/'xge_build_config.h']
  flags+=['-I'+str(v) for v in (root,root/'include',folder,engine/'host-xrt')]
  if kind=='static':flags+=['-DXGE_TEST_STATIC=1']
  elif win:flags+=['-DXGE_DLL','-DXUI_DLL']
  systems=['-l'+v for v in archive['system_libraries']] if kind=='static' else []
  if not win:systems+=['-pthread','-ldl','-lm','-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine)]
  for opt in ('O0','O2'):
   tag=kind+'-'+opt;binary=out/(tag+('.exe' if win else ''))
   assert not run(tag+'-build',[cc,*flags,'-'+opt,source,library,runtime,*systems,'-o',binary])
   for mode in range(16):run(tag+'-mode'+str(mode),[binary,font,str(mode)])
 report.update(passed=all(not v['exitCode'] for v in report['rows']),inputsUnchanged=all(sha(v)==h for v,h in inputs.items()))
 save();assert report['passed'] and report['inputsUnchanged']

if __name__=='__main__':main()
