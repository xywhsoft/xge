"""Matched production objects: Context/data/clipboard and manual bridge CPU.

Only a configuration-only OFFSCREEN/MANUAL xgeInit is used. No Sokol Run,
window, display, GPU, browser, network or actual clipboard is executed.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,time

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('root','fixtures','engine','runtime','out'):parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();root,fixtures,engine,runtime,out=(value.resolve() for value in (args.root,args.fixtures,args.engine,args.runtime,args.out))
    report=json.loads((engine/'build-report.json').read_text());assert report['linked'] and report['xrt_mode']=='host' and report['public_abi_schema']==2
    objects=[engine/'obj'/(name.replace('/','_').replace('\\','_')+'.o') for name in report['sources']]
    if 'XGE_ENABLE_HARFBUZZ' in report['features']:objects.append(root/'build/shared/obj/extlibs_harfbuzz_src_harfbuzz.cc.o')
    assert all(path.is_file() for path in objects)
    source=fixtures/'tests/xui/xui_transfer_context_test.c';helper=root/'tests/xui/xui_test_proxy.c'
    inputs={path for path in objects}|{source,helper,runtime,Path(__file__).resolve(),engine/'build-report.json',engine/'xge_build_config.h'}
    inputs.update(root/name for name in report['sources']);inputs.update((root/'include').glob('*.h'));inputs.update((engine/'host-xrt').glob('*'))
    inputs.update(root/name for name in ('src/xui/core/xui_internal.h','tests/xui/xui_owned_model_host_test.h','tests/xui/xui_test_proxy.h','extlibs/xrt/xrt.h'))
    sha=lambda path:hashlib.sha256(Path(path).read_bytes()).hexdigest();pins={str(path):sha(path) for path in sorted(inputs) if path.is_file()}
    out.mkdir(parents=True,exist_ok=False);env=dict(os.environ)
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    evidence=dict(complete=False,passed=False,scope=__doc__,inputs=pins,rows=[],staticProductionNativeObjects=True,actualClipboard=False)
    def save():(out/'results.json').write_text(json.dumps(evidence,indent=2)+'\n',encoding='utf-8')
    def run(name,command):
        start=time.monotonic();result=subprocess.run(list(map(str,command)),cwd=root,env=env,capture_output=True,timeout=180)
        log=out/(name+'.log');log.write_bytes(result.stdout+result.stderr);evidence['rows'].append(dict(name=name,command=list(map(str,command)),exitCode=result.returncode,seconds=time.monotonic()-start,logSHA256=sha(log)));save();print(name,result.returncode,flush=True)
        return result.returncode==0
    flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-parameter','-Wno-cast-function-type','-DXGE_XRT_HOST=1','-include',engine/'xge_build_config.h',*['-I'+str(path) for path in (root,root/'include',root/'tests/xui',engine/'host-xrt')]]
    system=['-lm','-lws2_32','-liphlpapi','-lgdi32','-luser32','-lshell32','-lole32','-loleaut32','-luuid','-limm32','-lwinmm','-lavrt'] if os.name=='nt' else ['-Wl,-rpath,'+str(runtime.parent),'-pthread','-ldl','-lm','-lX11','-lXi','-lXcursor','-lGL']
    save()
    for opt in ('O0','O2'):
        executable=out/(opt+('.exe' if os.name=='nt' else ''))
        if run(opt+'-build',['gcc',*flags,'-'+opt,source,helper,*objects,runtime,*system,'-o',executable]):run(opt+'-run',[executable])
    evidence.update(complete=True,inputsUnchanged=all(sha(path)==value for path,value in pins.items()))
    evidence['passed']=evidence['inputsUnchanged'] and len(evidence['rows'])==4 and all(row['exitCode']==0 for row in evidence['rows']);save();return 0 if evidence['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
