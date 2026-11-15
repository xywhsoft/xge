"""Qualify the unique CodeMarginModel lifetime foundation, not owned callbacks.

Complete immutable native SDK, O0/O2 hosted/static callers, original public
geometry/default/OOM tests, consumed failure and real CodeEdit/Context cleanup.
No xlang binding, window, device or network qualification.
"""
from pathlib import Path
import argparse,hashlib,json,os,re,shutil,subprocess,time


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('root','freeze','engine','static','runtime','out'):
        p.add_argument('--'+n,type=Path,required=True)
    p.add_argument('--fixtures',type=Path,help='Separately pinned QA sources; native SDK source freeze remains immutable.')
    a=p.parse_args();root,freeze,engine,static,runtime,out=[v.resolve()for v in
        (a.root,a.freeze,a.engine,a.static,a.runtime,a.out)]
    sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest();win=os.name=='nt'
    frozen=json.loads(freeze.read_text(encoding='utf-8'))
    assert all(sha(root/n)==h for n,h in frozen['files'].items())
    host=json.loads((engine/'build-report.json').read_text(encoding='utf-8'))
    st=json.loads((static/'build-report.json').read_text(encoding='utf-8'))
    assert host['linked']and host['xrt_mode']=='host'
    qualified=[]
    if host['reused']:
        assert host['compiled']==['src/xui/code/xui_code_margin.c']and len(host['reused'])==163
        parent=host['qualified_object_build'];old=Path(parent['report']);assert parent['inputs_unchanged']
        assert sha(old)==parent['report_sha256']
        old_report=json.loads(old.read_text(encoding='utf-8'))
        assert old_report['linked']and len(old_report['compiled'])==164 and not old_report['reused']
        qualified.append(old)
        for item in parent['objects']:
            for key,digest in (('object','sha256'),('dependency_file','dependency_sha256'),('cache_record','cache_sha256')):
                if key=='dependency_file'and item['source']=='Windows resource':
                    assert key not in item and digest not in item
                    continue
                path=Path(item[key]);assert sha(path)==item[digest];qualified.append(path)
    else:assert len(host['compiled'])==164
    assert st['passed']and st['sourceUnchanged']and st['xrt_mode']=='direct-external'
    image=engine/host['runtime_image'];library=engine/host['import_library']if win else image
    archive=static/st['archive'];tools={n:Path(shutil.which(n)).resolve()for n in ('gcc','g++')}
    fixtures=a.fixtures.resolve()if a.fixtures else root
    cases=[fixtures/'tests/xui'/n for n in ('xui_code_margin_lifetime_test.c','xui_code_margin_test.c','xui_constructor_retirement_test.c')]
    files=[Path(__file__).resolve(),freeze,*qualified,*tools.values(),*cases,fixtures/'tests/xui/xui_code_margin_host.h',
        image,library,archive,runtime,engine/'build-report.json',static/'build-report.json',
        engine/'xge_build_config.h',static/'xge_build_config.h',static/'xge_static_xrt_contract.h',
        *sorted((engine/'host-xrt').glob('*')),*sorted((root/'include').glob('*.h'))]
    inputs={str(v):sha(v)for v in files};out.mkdir(parents=True,exist_ok=False)
    snapshot=out/'input-snapshots';snapshot.mkdir()
    for v in (*cases,fixtures/'tests/xui/xui_code_margin_host.h',Path(__file__).resolve()):shutil.copy2(v,snapshot/v.name)
    r=dict(complete=False,passed=False,scope=__doc__,shippingDelivery=False,languageQualified=False,
        ownedMarginCallbacksImplemented=False,networkTests=False,inputs=inputs,rows=[],calibration=[],hot=[])
    def save():(out/'results.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf-8')
    env=os.environ.copy();key='PATH'if win else'LD_LIBRARY_PATH'
    env[key]=os.pathsep.join((str(engine),str(runtime.parent),env.get(key,'')))
    if win:
        tmp=out/'temp';tmp.mkdir();env.update(TEMP=str(tmp),TMP=str(tmp))
    def no_core():
        import resource
        resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    def run(name,cmd):
        cmd=list(map(str,cmd));start=time.monotonic()
        z=subprocess.run(cmd,cwd=root,env=env,capture_output=True,timeout=300,preexec_fn=None if win else no_core)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        r['rows'].append(dict(name=name,command=cmd,exitCode=z.returncode,seconds=time.monotonic()-start,logSha256=sha(log)))
        save();print(name,z.returncode,flush=True);assert not z.returncode,log;return z.stdout+z.stderr
    header=out/'header.c'
    header.write_text('''#include <xui.h>
int (*marginOwner)(xui_code_margin_model,bool*)=&xuiCodeMarginModelDestroyOwnerChecked;
int (*marginBegin)(xui_code_margin_model,xui_code_margin_scope_t*)=&xuiCodeMarginModelCallBegin;
int (*marginEnd)(xui_code_margin_scope_t*)=&xuiCodeMarginModelCallEnd;
bool (*marginBusy)(xui_code_margin_model)=&xuiCodeMarginModelOwnershipBusy;
bool (*marginTrace)(xui_code_margin_model,xrtownershipvisitor,void*)=&xuiCodeMarginModelTraceOwnership;
int (*marginWatch)(xui_code_margin_model,xui_lifetime_link_t*,xui_lifetime_invalidate_proc,void*)=&xuiCodeMarginModelLifetimeWatch;
''',encoding='utf-8')
    try:
        for kind,folder,lib in (('host',engine,library),('static',static,archive)):
            flags=['-Wall','-Wextra','-Werror','-Wno-cast-function-type','-Wno-unused-function',
                '-DXGE_XRT_HOST=1','-include',folder/'xge_build_config.h',
                *['-I'+str(v)for v in (root,root/'include',folder,engine/'host-xrt')]]
            flags+=['-DXGE_TEST_STATIC=1']if kind=='static'else['-DXGE_DLL=1','-DXUI_DLL=1']if win else[]
            systems=['-l'+v for v in st['system_libraries']]if kind=='static'else[]
            if not win:systems+=['-Wl,-rpath,'+str(engine),'-Wl,-rpath,'+str(runtime.parent),'-ldl','-lm','-pthread']
            for standard,tool,language in (('c11','gcc','c'),('c++11','g++','c++'),('c++17','g++','c++')):
                run(kind+'-header-'+standard,[tools[tool],'-std='+standard,*flags,'-x',language,'-fsyntax-only',header])
            for opt in ('O0','O2'):
                for label,case in zip(('lifetime','original','constructor'),cases):
                    name=kind+'-'+opt+'-'+label;binary=out/(name+('.exe'if win else''))
                    run(name+'-build',[tools['gcc'],'-std=c11',*flags,'-'+opt,case,lib,runtime,*systems,'-o',binary])
                    data=run(name,[binary,'10']if label=='constructor'else[binary])
                    if label=='constructor':
                        match=re.search(rb'prefixes=(\d+)',data);assert b'retirement passed'in data and match
                        r['calibration'].append(dict(kind=kind,opt=opt,prefixes=int(match[1]),completeContextHeap=True))
                    elif label=='lifetime':
                        assert b'Margin lifetime:'in data and b'1000 scopes,0 heap'in data
                        match=re.search(rb'public scope bytes=(\d+)',data);assert match
                        r['hot'].append(dict(kind=kind,opt=opt,rounds=1000,heapOperations=0,scopeBytes=int(match[1]),cpuEquivalenceClaim=False))
                    else:assert b'xui_code_margin_test passed'in data
        assert len(r['rows'])==30 and len(r['calibration'])==len(r['hot'])==4
        r.update(complete=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()),
            frozenSourcesUnchanged=all(sha(root/n)==h for n,h in frozen['files'].items()))
        r['passed']=r['inputsUnchanged']and r['frozenSourcesUnchanged'];assert r['passed']
    except Exception as error:r.update(complete=True,passed=False,error=str(error));raise
    finally:save()


if __name__=='__main__':main()
