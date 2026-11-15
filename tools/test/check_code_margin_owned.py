"""Qualify native owned Margin slots and real CodeEdit frame/event consumers.

No language/OOP or display-device qualification. Native O0/O2 hosted/static,
fault/reentry/heap balance plus unchanged numeric/lifetime/constructor/CodeEdit
regressions. Private snapshot probes link only the static archive.
"""
from pathlib import Path
import argparse,hashlib,json,os,re,shutil,subprocess,time

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('root','freeze','engine','static','runtime','fixtures','out'):
        p.add_argument('--'+n,type=Path,required=True)
    a=p.parse_args();root,freeze,engine,static,runtime,fixtures,out=[v.resolve()for v in
        (a.root,a.freeze,a.engine,a.static,a.runtime,a.fixtures,a.out)]
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();win=os.name=='nt'
    frozen=json.loads(freeze.read_text(encoding='utf-8'));assert all(sha(root/n)==h for n,h in frozen['files'].items())
    host=json.loads((engine/'build-report.json').read_text(encoding='utf-8'));st=json.loads((static/'build-report.json').read_text(encoding='utf-8'))
    assert host['linked']and host['xrt_mode']=='host'
    qualified=[]
    if host['reused']:
        assert host['compiled']==['src/xui/code/xui_code_edit.c']and len(host['reused'])==163
        parent=host['qualified_object_build'];old=Path(parent['report']);assert parent['inputs_unchanged']and sha(old)==parent['report_sha256']
        original=json.loads(old.read_text(encoding='utf-8'));assert len(original['compiled'])==164 and not original['reused']
        qualified.append(old)
        for item in parent['objects']:
            for key,digest in (('object','sha256'),('dependency_file','dependency_sha256'),('cache_record','cache_sha256')):
                if key=='dependency_file'and item['source']=='Windows resource':continue
                path=Path(item[key]);assert sha(path)==item[digest];qualified.append(path)
    else:assert len(host['compiled'])==164
    assert st['passed']and st['sourceUnchanged']and st['xrt_mode']=='direct-external'
    if st['reused']:
        assert len(st['compiled'])==2 and len(st['reused'])==164
        old=Path(st['reuse_parent']);original=json.loads(old.read_text(encoding='utf-8'))
        assert original['passed']and len(original['compiled'])==166 and not original['reused'];qualified.append(old)
    else:assert len(st['compiled'])==166
    image=engine/host['runtime_image'];library=engine/host['import_library']if win else image;archive=static/st['archive']
    out.mkdir(parents=True,exist_ok=False);qa=out/'qa';q=qa/'tests/xui';q.mkdir(parents=True)
    cases=['xui_code_margin_owned_test.c','xui_code_margin_lifetime_test.c','xui_code_margin_test.c','xui_constructor_retirement_test.c']
    files=[Path(__file__).resolve(),freeze,*qualified,image,library,archive,runtime,engine/'build-report.json',static/'build-report.json',
        *sorted((root/'include').glob('*.h')),*sorted((engine/'host-xrt').glob('*')),engine/'xge_build_config.h',
        static/'xge_build_config.h',static/'xge_static_xrt_contract.h',root/'src/xui/code/xui_code_margin_internal.h',
        *[fixtures/'tests/xui'/n for n in (*cases,'xui_code_margin_host.h')],
        *[root/'tests/xui'/n for n in ('xui_test_proxy.c','xui_test_proxy.h','xui_code_edit_test.c')]]
    for n in (*cases,'xui_code_margin_host.h'):shutil.copy2(fixtures/'tests/xui'/n,q/n)
    for n in ('xui_test_proxy.c','xui_test_proxy.h','xui_code_edit_test.c'):shutil.copy2(root/'tests/xui'/n,q/n)
    wrapper=q/'original_edit.c';wrapper.write_text('''#include "xui_code_margin_host.h"
#define main marginOriginalEditMain
#include "xui_code_edit_test.c"
#undef main
#include "xui_test_proxy.c"
int main(void){marginBind();return marginOriginalEditMain();}
''',encoding='utf-8')
    header=qa/'prototype.c';header.write_text('''#include <xui.h>
int (*renderSlot)(xui_code_margin_model,int,xui_code_margin_render_proc,void*,const xui_callback_owner_t*)=&xuiCodeMarginModelSetRenderCallbackOwned;
int (*eventSlot)(xui_code_margin_model,int,xui_code_margin_event_proc,void*,const xui_callback_owner_t*)=&xuiCodeMarginModelSetEventCallbackOwned;
int (*state)(xui_code_margin_model,int,xui_code_margin_state_t*)=&xuiCodeMarginModelGetState;
''',encoding='utf-8')
    tools={n:Path(shutil.which(n)).resolve()for n in ('gcc','g++')};files+=list(tools.values())
    inputs={str(v):sha(v)for v in files}
    r=dict(complete=False,passed=False,shippingDelivery=False,languageQualified=False,networkTests=False,inputs=inputs,rows=[],hot=[],constructor=[])
    env=os.environ.copy();key='PATH'if win else'LD_LIBRARY_PATH'
    env[key]=os.pathsep.join((str(engine),str(runtime.parent),env.get(key,'')))
    if win:
        tmp=out/'temp';tmp.mkdir();env.update(TEMP=str(tmp),TMP=str(tmp))
    def save():(out/'results.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf-8')
    def no_core():
        import resource
        resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    def run(name,cmd):
        cmd=list(map(str,cmd));start=time.monotonic()
        z=subprocess.run(cmd,cwd=qa,env=env,capture_output=True,timeout=300,preexec_fn=None if win else no_core)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        r['rows'].append(dict(name=name,command=cmd,exitCode=z.returncode,seconds=time.monotonic()-start,logSha256=sha(log)))
        save();print(name,z.returncode,flush=True);assert not z.returncode,log;return z.stdout+z.stderr
    try:
        for kind,folder,lib in (('host',engine,library),('static',static,archive)):
            flags=['-Wall','-Wextra','-Werror','-Wno-cast-function-type','-Wno-unused-function',
                '-DXGE_XRT_HOST=1','-include',folder/'xge_build_config.h',
                *['-I'+str(v)for v in (qa,q,root,root/'include',root/'src/xui/core',folder,engine/'host-xrt')]]
            flags+=['-DXGE_TEST_STATIC=1']if kind=='static'else['-DXGE_DLL=1','-DXUI_DLL=1']if win else[]
            systems=['-l'+v for v in st['system_libraries']]if kind=='static'else[]
            if not win:systems+=['-Wl,-rpath,'+str(engine),'-Wl,-rpath,'+str(runtime.parent),'-ldl','-lm','-pthread']
            for std,tool,language in (('c11','gcc','c'),('c++11','g++','c++'),('c++17','g++','c++')):
                run(kind+'-header-'+std,[tools[tool],'-std='+std,*flags,'-x',language,'-fsyntax-only',header])
            for opt in ('O0','O2'):
                for label,case in zip(('owned','lifetime','original','constructor','editor'),[*[q/n for n in cases],wrapper]):
                    name=kind+'-'+opt+'-'+label;exe=out/(name+('.exe'if win else''))
                    run(name+'-build',[tools['gcc'],'-std=c11',*flags,'-'+opt,case,lib,runtime,*systems,'-o',exe])
                    data=run(name,[exe,'10']if label=='constructor'else[exe])
                    if label=='owned':
                        assert b'owned Margin passed'in data
                        if kind=='static':
                            assert b'1000 snapshots/invocations,0 heap,0 external Hold/Drop'in data
                            r['hot'].append(dict(kind=kind,opt=opt,rounds=1000,heapOperations=0,externalOwnerCalls=0,cpuEquivalenceClaim=False))
                    if label=='constructor':
                        match=re.search(rb'prefixes=(\d+)',data);assert match
                        r['constructor'].append(dict(kind=kind,opt=opt,prefixes=int(match[1])))
        assert len(r['rows'])==46 and len(r['hot'])==2
        r.update(complete=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()),frozenSourcesUnchanged=all(sha(root/n)==h for n,h in frozen['files'].items()))
        r['passed']=r['inputsUnchanged']and r['frozenSourcesUnchanged'];assert r['passed']
    except Exception as error:r.update(complete=True,passed=False,error=str(error));raise
    finally:save()

if __name__=='__main__':main()
