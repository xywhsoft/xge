"""Radio init failure cleanup against matched hosted/static O2 SDKs.
Native C O0/O2 allocation-prefix tests plus the unchanged radio regression. No language/window/network or shipping-delivery certification.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,time

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ('root','test-root','engine','static','runtime','out'):p.add_argument('--'+n,type=Path,required=True)
 a=p.parse_args();root,tests,engine,static,runtime,out=[v.resolve()for v in(a.root,a.test_root,a.engine,a.static,a.runtime,a.out)]
 win=os.name=='nt';cc=Path(shutil.which('gcc')).resolve();sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
 h=json.loads((engine/'build-report.json').read_text(encoding='utf-8'));s=json.loads((static/'build-report.json').read_text(encoding='utf-8'))
 assert h['linked']and h['xrt_mode']=='host'and s['passed']and s['sourceUnchanged']
 image=engine/h['runtime_image'];library=engine/h['import_library']if win else image;archive=static/s['archive']
 source=tests/'tests/xui/xui_radio_init_failure_test.c';legacy=root/'tests/xui/xui_radio_test.c'
 helper=source
 files=[Path(__file__),cc,source,helper,legacy,image,library,archive,runtime,engine/'build-report.json',static/'build-report.json',engine/'xge_build_config.h',static/'xge_build_config.h',static/'xge_static_xrt_contract.h',*sorted((engine/'host-xrt').glob('*'))]
 files.extend(root/n for n in ('src/xui/controls/xui_radio.c','src/xui/core/xui_input.c','src/xui/core/xui_core.c','src/xui/core/xui_callback_owned.c','src/xui/core/xui_internal.h','src/xui/core/xui_widget.c','tests/xui/xui_test_proxy.c','tests/xui/xui_test_proxy.h','extlibs/xrt/xrt.h','extlibs/xrt/xrt_config.h'))
 files.extend(sorted((root/'include').glob('*.h')));inputs={str(f):sha(f)for f in files};out.mkdir(parents=True,exist_ok=False);shutil.copy2(source,out/'testcase.c')
 report=dict(complete=False,passed=False,scope=__doc__,shippingDelivery=False,languageBindingsQualified=False,networkTests=False,inputs=inputs,rows=[])
 def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
 env=dict(os.environ);key='PATH'if win else'LD_LIBRARY_PATH';env[key]=os.pathsep.join((str(engine),str(runtime.parent),env.get(key,'')))
 def no_core():
  import resource
  resource.setrlimit(resource.RLIMIT_CORE,(0,0))
 def run(name,cmd):
  cmd=list(map(str,cmd));start=time.monotonic();z=subprocess.run(cmd,cwd=root,env=env,capture_output=True,timeout=120,preexec_fn=None if win else no_core)
  log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr);report['rows'].append(dict(name=name,command=cmd,exitCode=z.returncode,seconds=time.monotonic()-start,logSha256=sha(log)));save();print(name,z.returncode,flush=True);assert not z.returncode,log;return z.stdout+z.stderr
 try:
  for kind,folder,lib in(('host',engine,library),('static',static,archive)):
   flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-Wno-unused-function','-DXGE_XRT_HOST=1','-include',folder/'xge_build_config.h',*['-I'+str(v)for v in(root,root/'include',folder,engine/'host-xrt')]]
   flags+=['-DXGE_TEST_STATIC=1']if kind=='static'else['-DXGE_DLL=1','-DXUI_DLL=1']if win else[]
   systems=['-l'+n for n in s['system_libraries']]if kind=='static'else[]
   if not win:systems+=['-Wl,-rpath,'+str(engine),'-Wl,-rpath,'+str(runtime.parent),'-ldl','-lm','-pthread']
   # The legacy public suite has no host bootstrap. Wrap its unchanged main.
   prefix=helper.read_text(encoding='utf-8').split('static size_t backendReferences=1;')[0]
   wrapper=out/(kind+'-legacy.c');body='''
int main(void){
#if defined(XGE_TEST_STATIC)
 const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
#else
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
 return original_input_main();
}
'''
   wrapper.write_text(prefix+'\n#define main original_input_main\n#include "'+legacy.as_posix()+'"\n#undef main\n'+body,encoding='utf-8')
   for opt in('O0','O2'):
    for label,case in(('owned',source),('legacy',wrapper)):
     name=kind+'-'+opt+'-'+label;binary=out/(name+('.exe'if win else''));run(name+'-build',[cc,*flags,'-'+opt,case,lib,runtime,*systems,'-o',binary]);data=run(name,[binary])
     if label=='owned':assert b'Radio init failure: 192 allocation prefixes, cleared outputs, parent state and exact heap cleanup passed'in data
  report.update(complete=True,inputsUnchanged=all(sha(f)==v for f,v in inputs.items()));report['passed']=report['inputsUnchanged'];assert report['passed']
 except Exception as e:report.update(complete=True,passed=False,error=str(e));raise
 finally:save()
if __name__=='__main__':main()
