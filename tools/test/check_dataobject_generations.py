"""CPU-only DataObject generation tests against exact native production objects."""
from pathlib import Path
import argparse, hashlib, json, os, shutil, subprocess, time

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('source','engine','runtime','out'):parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--fixture-root',type=Path)
    args=parser.parse_args()
    source,engine,runtime,out=[p.resolve() for p in (args.source,args.engine,args.runtime,args.out)]
    out.mkdir(parents=True,exist_ok=False)
    build=json.loads((engine/'build-report.json').read_text())
    sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    objects=[engine/'obj'/(name.replace('/','_')+'.o') for name in build['sources']]
    objects+=list((source/'build/shared/obj').glob('*.o'))
    if os.name=='nt':objects+=list((engine/'obj').glob('*_res.o'))
    fixtures=args.fixture_root.resolve() if args.fixture_root else source/'tests/xui'
    test=fixtures/'xui_data_object_generation_test.c'
    legacy=[fixtures/'legacy/test_drag_data_object.c',fixtures/'legacy/xui_drag_drop_test.c']
    pins=[test,*legacy,fixtures/'xui_owned_model_host_test.h',source/'src/xge/input/xge_drag_drop.c',source/'src/xui/core/xui_drag_drop.c',source/'include/xui.h',source/'include/xge.h',runtime,engine/'build-report.json',Path(__file__).resolve(),*objects,*list((engine/'host-xrt').glob('*'))]
    inputs={str(p):sha(p) for p in pins if p.is_file()}
    report=dict(complete=False,passed=False,scope=__doc__,networkTests=False,desktopTests=False,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,cmd,env=None):
        start=time.monotonic();q=subprocess.run(list(map(str,cmd)),cwd=source,env=env,capture_output=True,timeout=120)
        log=out/(name+'.log');log.write_bytes(q.stdout+q.stderr)
        report['rows'].append(dict(name=name,command=list(map(str,cmd)),exitCode=q.returncode,seconds=time.monotonic()-start,logSHA=sha(log)));save();print(name,q.returncode,flush=True)
        return q.returncode==0
    flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-cast-function-type','-DXGE_XRT_HOST=1','-include',engine/'xge_build_config.h']
    flags+=['-I'+str(p) for p in (source,source/'include',fixtures,engine/'host-xrt')]
    libs=['-lm']+(['-lws2_32','-liphlpapi','-lgdi32','-luser32','-lshell32','-lole32','-loleaut32','-luuid','-limm32','-lwinmm','-lavrt'] if os.name=='nt' else ['-lX11','-lXi','-lXcursor','-lGL','-ldl','-lpthread','-Wl,-rpath,'+str(runtime.parent)])
    env=dict(os.environ)
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    save()
    for opt in ('O0','O2'):
        exe=out/('generations-'+opt+('.exe' if os.name=='nt' else ''))
        if run(opt+'-build',[build['compiler'][0],*flags,'-'+opt,test,*objects,runtime,*libs,'-o',exe]):run(opt+'-run',[exe],env)
    for original in legacy:
        assert original.is_file(),original
        text=original.read_text(encoding='utf-8').replace('#include "single/xge.c"','#include "xge.h"')
        assert text.count('int main(void)')==1
        text='#include "xui_owned_model_host_test.h"\n'+text.replace('int main(void)','int dataLegacyMain(void)',1)+'\nint main(void){modelHostBind();return dataLegacyMain();}\n'
        wrapper=out/(original.stem+'-host.c');wrapper.write_text(text)
        for opt in ('O0','O2'):
            name=original.stem+'-'+opt;exe=out/(name+('.exe' if os.name=='nt' else ''))
            if run(name+'-build',[build['compiler'][0],*flags,'-'+opt,wrapper,*objects,runtime,*libs,'-o',exe]):run(name+'-run',[exe],env)
    report['complete']=True;report['inputsUnchanged']=all(sha(Path(p))==value for p,value in inputs.items());report['passed']=report['inputsUnchanged'] and len(report['rows'])==12 and all(not row['exitCode'] for row in report['rows']);save()
    return 0 if report['passed'] else 1

if __name__=='__main__':raise SystemExit(main())
