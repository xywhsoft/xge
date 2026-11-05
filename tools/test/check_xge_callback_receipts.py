"""CPU-only acceptance of the actual global publisher header with one XRT host."""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,time

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','runtime','out'):p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--kernel-header',type=Path,help='Isolated historical kernel for a rejection reproduction')
    p.add_argument('--expect-rejection',action='store_true')
    a=p.parse_args();root=a.root.resolve();runtime=a.runtime.resolve();out=a.out.resolve();out.mkdir(exist_ok=False)
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    paths=[Path(__file__).resolve(),root/'tests/xge/xge_owned_callback_receipt_test.c',root/'src/xge/core/xge_callback_owned.h',root/'include/xge.h',root/'include/xge_config.h',root/'extlibs/xrt/xrt_config.h',root/'extlibs/xrt/xrt.h',runtime]
    inputs={str(f):sha(f)for f in paths};report=dict(complete=False,passed=False,shippingDelivery=False,fullEngineBuild=False,scope=__doc__,inputs=inputs,rows=[])
    include=[]
    if a.kernel_header:
        header=a.kernel_header.resolve();inputs[str(header)]=sha(header);target=out/'override/src/xge/core/xge_callback_owned.h';target.parent.mkdir(parents=True);shutil.copy2(header,target);include=['-I'+str(out/'override')]
    report['expectedKernelRejection']=a.expect_rejection
    env=dict(os.environ)
    if os.name=='nt':shutil.copy2(runtime,out/runtime.name)
    else:env['LD_LIBRARY_PATH']=str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    try:
        for opt in ('O0','O2'):
            target=out/(opt+('.exe'if os.name=='nt'else''));system=['-lws2_32','-lole32','-ladvapi32','-lshell32','-liphlpapi']if os.name=='nt'else['-pthread','-ldl','-lm']
            command=['gcc','-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-'+opt,*include,'-I'+str(root/'include'),'-I'+str(root),str(paths[1]),str(runtime),*system,'-o',str(target)]
            for name,args in [(opt+'-build',command),(opt+'-run',[str(target)])]:
                start=time.monotonic();result=subprocess.run(args,env=env,capture_output=True,timeout=180);log=out/(name+'.log');log.write_bytes(result.stdout+result.stderr);negative=a.expect_rejection and name.endswith('-run');report['rows'].append(dict(name=name,exitCode=result.returncode,expectedFailure=negative,command=args,seconds=round(time.monotonic()-start,3),logSha256=sha(log)));save();print(name,result.returncode,flush=True);assert bool(result.returncode)==negative,log
        report.update(complete=True,passed=True)
    except Exception as error:report['error']=str(error);raise
    finally:report['inputsUnchanged']=all(sha(f)==s for f,s in inputs.items());save()
if __name__=='__main__':main()
