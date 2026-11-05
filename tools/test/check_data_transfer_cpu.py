"""Exact production transfer consumers with typed CPU-only OS boundaries.

No engine/window/GPU/browser/network or actual clipboard is initialized.
Windows replaces typed OLE publication/registration calls in the production
single-XGE translation unit; Linux replaces X11 selection calls. This does
not replace matching SDK, resident capsule DLL lifetime or language gates.
"""
from pathlib import Path
import argparse,hashlib,json,os,shlex,shutil,subprocess,time

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('root','runtime','out'):parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();root,runtime,out=(value.resolve() for value in (args.root,args.runtime,args.out))
    out.mkdir(parents=True,exist_ok=False);sha=lambda path:hashlib.sha256(path.read_bytes()).hexdigest()
    paths={Path(__file__).resolve(),runtime,root/'config/includes.rsp',root/'config/modules.json',root/'single/xge.c'}
    for folder in ('include','src/xge','src/xui/core','extlibs'):
        paths.update(path for path in (root/folder).rglob('*') if path.is_file() and path.suffix.lower() in ('.h','.c','.inl'))
    tests=['tests/xui/xui_data_snapshot_test.c','tests/xge/xge_transfer_clipboard_cpu_test.c' if os.name=='nt' else 'tests/xge/xge_transfer_x11_cpu_test.c']
    paths.update(root/name for name in tests);inputs={str(path):sha(path) for path in sorted(paths)}
    env=dict(os.environ)
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    report=dict(complete=False,passed=False,scope=__doc__,inputs=inputs,rows=[],actualClipboard=False,gpuTests=False,networkTests=False)
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    def run(name,command):
        start=time.monotonic();result=subprocess.run(list(map(str,command)),cwd=root,env=env,capture_output=True,timeout=180)
        log=out/(name+'.log');log.write_bytes(result.stdout+result.stderr)
        report['rows'].append(dict(name=name,command=list(map(str,command)),exitCode=result.returncode,seconds=time.monotonic()-start,logSHA256=sha(log)))
        save();print(name,result.returncode,flush=True);return result.returncode==0
    cc=shutil.which('gcc');assert cc
    includes=shlex.split((root/'config/includes.rsp').read_text())
    flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-parameter','-Wno-cast-function-type']
    system=['-lole32','-loleaut32','-luuid','-luser32','-lgdi32','-lshell32','-limm32','-lcomdlg32','-ldwmapi','-lopengl32'] if os.name=='nt' else ['-lX11','-lXi','-lXcursor','-lGL','-ldl','-pthread','-lm']
    save()
    for opt in ('O0','O2'):
        for index,test in enumerate(tests):
            name=('snapshot' if index==0 else 'platform')+'-'+opt
            program=out/(name+('.exe' if os.name=='nt' else ''))
            extra=['-DXRT_MODULE_ALL='] if index==0 else ['-DXGE_XRT_HOST=1','-ffunction-sections','-fdata-sections']
            libraries=[] if index==0 and os.name=='nt' else ['-pthread','-lm'] if index==0 else system
            command=[cc,*flags,'-'+opt,*includes,*extra,root/test,runtime,*libraries]
            if index==1:command+=['-Wl,--gc-sections']
            command+=['-o',program]
            if run(name+'-build',command):run(name+'-run',[program])
    report.update(complete=True,inputsUnchanged=all(sha(Path(name))==value for name,value in inputs.items()))
    report['passed']=report['inputsUnchanged'] and len(report['rows'])==8 and all(row['exitCode']==0 for row in report['rows']);save()
    return 0 if report['passed'] else 1

if __name__=='__main__':raise SystemExit(main())
