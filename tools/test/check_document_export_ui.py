"""Exercise full-engine Document accessibility and MessageList export consumers.

Uses the explicit frozen engine, matching hosted binary and one resident XRT.
No GPU, network or desktop mutation. This does not qualify final IDE delivery.
"""
from pathlib import Path
import argparse,hashlib,json,os,shlex,shutil,subprocess,time

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('root','host','runtime','runtime-headers','out'):p.add_argument('--'+n,type=Path,required=True)
    a=p.parse_args();root,host,runtime,headers,out=[v.resolve()for v in(a.root,a.host,a.runtime,a.runtime_headers,a.out)]
    out.mkdir(parents=True,exist_ok=False);(out/'build').mkdir();win=os.name=='nt';sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();cc=Path(shutil.which('gcc')).resolve()
    d=json.loads((host/'build-report.json').read_text());assert (host/('xge3_host.dll'if win else'libxge3_host.so')).is_file()
    helper=out/'host.c';helper.write_text('''#include <xrt_decl.h>
#include <xge_host_xrt_contract.h>
#include <xl6_host_xrt.h>
#include <assert.h>
#include <stdio.h>
extern int case_main(void);
static xge_host_xrt_proc resolve(void* u,const char* n){(void)u;return xl6HostXrtResolveV1(n);}
static bool dump(const xmemdebugallocation* v,void* u){
 (void)u;fprintf(stderr,"Live XRT allocation: %zu bytes %s:%u\\n",v->Size,v->File?v->File:"<native>",v->Line);
 if(v->Size){const unsigned char* p=v->Address;size_t n=v->Size<128?v->Size:128;for(size_t i=0;i<n;++i)fprintf(stderr,"%02x",p[i]);fputc('\\n',stderr);}
 return true;
}
int main(void){
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
 const char* missing=NULL;assert(xgeHostXrtBind(&contract,resolve,NULL,&missing)==0);
 assert(xrtMemDebugEnable(true));xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);
 int result=case_main();
 if(xrtGetError()){puts("Released final caller-owned native diagnostic");xrtClearError();}
 xrtMemDebugSnapshot(&b);
 assert(a.InvalidFreeCount==b.InvalidFreeCount && a.DoubleFreeCount==b.DoubleFreeCount);
 printf("UI XRT live allocation delta: %lld; invalid/double frees: zero\\n",(long long)b.LiveCount-(long long)a.LiveCount);
 if(a.LiveCount!=b.LiveCount || a.LiveBytes!=b.LiveBytes)xrtMemDebugVisitLive(dump,NULL);
 assert(a.LiveCount==b.LiveCount && a.LiveBytes==b.LiveBytes);
 return result;
}
''')
    inputs={str(v):sha(v)for v in(cc,Path(__file__),runtime,host/'build-report.json',helper)};rows=[]
    report=dict(complete=False,passed=False,scope=__doc__,inputs=inputs,rows=rows,shippingDelivery=False)
    save=lambda:(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    env=dict(os.environ)
    if win:
        shutil.copy2(runtime,out/runtime.name);shutil.copy2(host/'xge3_host.dll',out/'xge3_host.dll')
    else:env['LD_LIBRARY_PATH']=str(host)+':'+str(runtime.parent)+':'+env.get('LD_LIBRARY_PATH','')
    def run(name,cmd):
        cmd=list(map(str,cmd));actual=cmd
        if cmd[0]==str(cc):
            rsp=out/(name+'.rsp');rsp.write_text('\n'.join('"'+v.replace('\\','/').replace('"','\\"')+'"'for v in cmd[1:])+'\n');actual=[cmd[0],'@'+str(rsp)]
        start=time.monotonic();z=subprocess.run(actual,cwd=out,env=env,capture_output=True,timeout=240);log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr);rows.append(dict(name=name,command=cmd,exitCode=z.returncode,seconds=time.monotonic()-start,logSha256=sha(log)));save();print(name,z.returncode,flush=True);assert not z.returncode,log
    try:
        includes=['-I'+str(root/v[2:])for v in shlex.split((root/'config/includes.rsp').read_text())if v.startswith('-I')]
        includes=['-I'+str(host),'-I'+str(host/'host-xrt'),'-I'+str(headers),'-I'+str(headers/'__xl_runtime'),*includes,'-I'+str(root/'tests/xui')]
        flags=['-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-parameter','-DXGE_DLL','-DXUI_DLL',*includes]
        libs=[host/'xge3_host.lib',runtime,'-lm','-lws2_32','-liphlpapi','-lgdi32','-luser32','-lshell32','-lole32','-loleaut32','-luuid','-lwinmm','-lavrt']if win else[host/'libxge3_host.so',runtime,'-pthread','-ldl','-lm']
        for name in ('xui_document_accessible_text_test','xui_message_document_test','xui_message_list_test'):
            sources=[root/'tests/xui'/(name+'.c'),root/'tests/xui/xui_test_proxy.c']
            if name=='xui_document_accessible_text_test':sources.append(root/'src/xui/core/xui_unicode.c')
            objects=[]
            for source in sources:
                inputs[str(source)]=sha(source);obj=out/(name+'-'+source.stem+'.o');objects.append(obj)
                run(name+'-'+source.stem,[cc,*flags,'-Dmain=case_main','-c',source,'-o',obj])
            program=out/(name+('.exe'if win else''))
            run(name+'-build',[cc,*flags,helper,*objects,*libs,'-o',program]);run(name+'-execute',[program])
        report.update(complete=True,passed=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()));assert report['inputsUnchanged']
    except Exception as e:report.update(complete=True,error=str(e));raise
    finally:save()

if __name__=='__main__':main()
