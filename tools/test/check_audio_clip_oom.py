"""Valid CPU WAV decode, optional shrink OOM and immutable PCM span checks.

No audio engine/device, window, graphics or network. --baseline is an expected
old failure control, never release qualification.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('root','test-root','engine','static','runtime','host-header','out'):p.add_argument('--'+name,type=Path,required=True)
 p.add_argument('--baseline',action='store_true');p.add_argument('--case',choices=('oom','span','all'),default='oom');a=p.parse_args()
 assert not a.baseline or a.case=='oom'
 root,tests,engine,static,runtime,header,out=[v.resolve() for v in (a.root,a.test_root,a.engine,a.static,a.runtime,a.host_header,a.out)]
 sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest()
 canonical=lambda f:hashlib.sha256(Path(f).read_bytes().replace(b'\r\n',b'\n')).hexdigest()
 host=json.loads((engine/'build-report.json').read_text());archive=json.loads((static/'build-report.json').read_text())
 assert host['linked'] and host['xrt_mode']=='host' and archive['passed'] and archive['sourceUnchanged']
 assert host['features']==archive['features'] and host['xrt_host_header_sha256']==archive['xrt_header_sha256']==canonical(header)==canonical(root/'extlibs/xrt/xrt.h')
 win=os.name=='nt';cc=Path(shutil.which('gcc')).resolve();image=engine/host['runtime_image'];lib=engine/host['import_library'] if win else image
 cases=('oom','span') if a.case=='all' else (a.case,)
 sources={name:tests/'tests/xge'/('xge_audio_clip_'+name+'_test.c') for name in cases}
 files=[Path(__file__).resolve(),*sources.values(),cc,runtime,header,image,lib,engine/'build-report.json',static/'build-report.json',static/archive['archive'],static/'xge_build_config.h',static/'xge_static_xrt_contract.h',engine/'xge_build_config.h',root/'src/xge/audio/xge_audio_clip.inc']
 files.extend((root/'include').glob('*.h'));files.extend(v for v in (engine/'host-xrt').iterdir() if v.is_file())
 inputs={str(v):sha(v) for v in files};out.mkdir(parents=True,exist_ok=False);env=dict(os.environ)
 if win:
  for v in (image,runtime):shutil.copy2(v,out/v.name)
 else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+str(engine)+':'+env.get('LD_LIBRARY_PATH','')
 report=dict(passed=False,scope=__doc__,cases=cases,baseline=a.baseline,shippingDelivery=False,audioDevice=False,networkTests=False,inputs=inputs,rows=[])
 def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
 def run(name,args,discover=False):
  args=list(map(str,args));r=subprocess.run(args,cwd=out,env=env,capture_output=True,timeout=90)
  log=out/(name+'.log');log.write_bytes(r.stdout+r.stderr)
  report['rows'].append(dict(name=name,command=args,exitCode=r.returncode,logSha256=sha(log)));save();print(name,r.returncode,flush=True)
  if not discover:assert not r.returncode,log
 variants=[('host',engine,lib)] if a.baseline else [('host',engine,lib),('static',static,static/archive['archive'])]
 for kind,folder,library in variants:
  flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-DXGE_XRT_HOST=1','-include',folder/'xge_build_config.h']
  flags+=['-I'+str(v) for v in (root,root/'include',folder,engine/'host-xrt')]
  if kind=='static':flags+=['-DXGE_TEST_STATIC=1']
  elif win:flags+=['-DXGE_DLL','-DXUI_DLL']
  systems=['-l'+v for v in archive['system_libraries']] if kind=='static' else []
  if not win:systems+=['-pthread','-ldl','-lm','-Wl,-rpath,'+str(runtime.parent),'-Wl,-rpath,'+str(engine)]
  for opt in (('O2',) if a.baseline else ('O0','O2')):
   for name,source in sources.items():
    tag=kind+'-'+opt+'-'+name;binary=out/(tag+('.exe' if win else ''))
    run(tag+'-build',[cc,*flags,'-'+opt,source,library,runtime,*systems,'-o',binary])
    run(tag+'-run',[binary],discover=a.baseline)
 report.update(passed=not a.baseline and all(not v['exitCode'] for v in report['rows']),inputsUnchanged=all(sha(v)==h for v,h in inputs.items()))
 assert report['inputsUnchanged'];save()
 if a.baseline:assert any(v['exitCode'] for v in report['rows'])
 else:assert report['passed']

if __name__=='__main__':main()
