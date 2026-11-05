"""Exact standalone production DataObject C components, no Widget/device backend.

This is Native CPU subset acceptance, not a full matching DLL/SDK, resident
capsule, final binding, clipboard or runtime-release acceptance.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,time

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('root','runtime','out'):parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();root,runtime,out=[p.resolve() for p in (args.root,args.runtime,args.out)]
    out.mkdir(parents=True,exist_ok=False);sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    names=['tests/xui/xui_owned_data_object_test.c','src/xge/input/xge_data_object.c','src/xui/core/xui_data_object.c','src/xge/core/xge_data_lifetime_internal.h','src/xge/core/xge_data_lifetime.inl','include/xge.h','include/xui.h','include/xnative_transfer.h','extlibs/xrt/xrt.h','tests/xui/xui_data_object_generation_test.c','tests/xui/xui_owned_model_host_test.h']
    inputs={str(root/n):sha(root/n) for n in names};inputs[str(runtime)]=sha(runtime);inputs[str(Path(__file__).resolve())]=sha(Path(__file__).resolve())
    report=dict(complete=False,passed=False,scope=__doc__,inputs=inputs,rows=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,command,env=None):
        start=time.monotonic();q=subprocess.run(list(map(str,command)),cwd=root,env=env,capture_output=True,timeout=60)
        log=out/(name+'.log');log.write_bytes(q.stdout+q.stderr);report['rows'].append(dict(name=name,command=list(map(str,command)),exitCode=q.returncode,seconds=time.monotonic()-start,logSHA256=sha(log)));save();print(name,q.returncode,flush=True);return not q.returncode
    env=dict(os.environ)
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-cast-function-type','-DXRT_MODULE_ALL=','-I.','-Iinclude','-Isrc/xge/core','-Itests/xui']
    library=[] if os.name=='nt' else ['-lm','-pthread','-Wl,-rpath,'+str(runtime.parent)]
    save()
    for opt in ('O0','O2'):
        exe=out/('owned-'+opt+('.exe' if os.name=='nt' else ''))
        command=['gcc',*flags,'-'+opt,root/names[0],root/names[1],root/names[2],runtime,*library,'-o',exe]
        if run(opt+'-build',command):run(opt+'-run',[exe],env)
    for opt in ('O0','O2'):
        exe=out/('raw-'+opt+('.exe' if os.name=='nt' else ''))
        command=['gcc',*flags,'-'+opt,root/'tests/xui/xui_data_object_generation_test.c',root/names[1],root/names[2],runtime,*library,'-o',exe]
        if run('raw-'+opt+'-build',command):run('raw-'+opt+'-run',[exe],env)
    header=out/'headers.c';header.write_text('#include "xge.h"\n#include "xui.h"\n#include "xnative_transfer.h"\nstatic bool native_trace(const void* p,xrtownershipvisitor v,void* u){(void)p;(void)v;(void)u;return true;}\nxn_transfer_native_owner_v1 owner={sizeof(owner),XN_TRANSFER_NATIVE_OWNER_V1_TAG,0,NULL,native_trace,NULL};\n')
    for standard,cc in [('c11','gcc'),('c++11','g++'),('c++17','g++')]:
        run('header-'+standard,[cc,'-std='+standard,'-Wall','-Wextra','-Werror','-I.','-Iinclude','-fsyntax-only',header])
    report['complete']=True;report['inputsUnchanged']=all(sha(Path(p))==v for p,v in inputs.items());report['passed']=report['inputsUnchanged'] and len(report['rows'])==11 and all(not r['exitCode'] for r in report['rows']);save();return 0 if report['passed'] else 1

if __name__=='__main__':raise SystemExit(main())
