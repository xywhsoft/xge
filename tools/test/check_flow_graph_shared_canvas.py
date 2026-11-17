"""CPU canvas handoff using exact static SDK plus replacement production units.

Freezes the qualified baseline, excludes unrelated worktree text/geometry, and
links one compiler-owned matching XRT. This does not rebuild/install the SDK,
host image or language packages and performs no graphics/network operation.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,time


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in('root','baseline-source','static-sdk','runtime-dir','compiler-report','out'):
        parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args()
    root,baseline,sdk,runtime,build,out=[p.resolve()for p in
        (args.root,args.baseline_source,args.static_sdk,args.runtime_dir,args.compiler_report,args.out)]
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    win=os.name=='nt';cc=Path(shutil.which('gcc')).resolve()
    native_report=sdk/'build-report.json';native=json.loads(native_report.read_text())
    metadata=json.loads(build.read_text())
    assert native['complete'] and native['passed'] and native['xrt_mode']=='direct-external'
    assert metadata['complete'] and metadata['passed'] and metadata['fullTccXrtAndVfs'] and not metadata['changedInputs']
    archive=sdk/native['archive'];assert sha(archive)==native['archive_sha256']
    assert sha(baseline/'extlibs/xrt/xrt.h')==native['xrt_header_sha256']==sha(Path(metadata['source'])/'lib/xrt/xrt.h')
    products=[runtime/name for name in(('xl.exe','xl.dll','xlc.dll')if win else('xl','libxl.so','libxlc.so'))]
    for product in products:assert sha(product)==metadata['products'][product.name]['sha256']
    overlays=['include/xui.h','src/xui/core/xui_internal.h','src/xui/workflow/xui_flow_graph.c',
        'src/xui/workflow/xui_flow_graph_owned.inl','src/xui/workflow/xui_flow_graph_widget.c',
        'src/xui/workflow/xui_flow_graph_widget_internal.h','src/xui/workflow/xui_workflow.c',
        'src/xui/workflow/xui_workflow_owned.inl','src/xge/core/xge_data_lifetime.inl',
        'tests/xui/xui_flow_graph_shared_widget_test.c']
    pins=[Path(__file__).resolve(),cc,build,native_report,archive,sdk/'xge_build_config.h',*products,*[root/r for r in overlays]]
    inputs={str(p):sha(p)for p in pins}
    out.mkdir(parents=True,exist_ok=False);source=out/'source'
    assert not out.is_relative_to(root) and not out.is_relative_to(baseline)
    def link(src,dst):
        try:os.link(src,dst);return dst
        except OSError:return shutil.copy2(src,dst)
    shutil.copytree(baseline,source,copy_function=link)
    for rel in overlays:
        target=source/rel;target.parent.mkdir(parents=True,exist_ok=True)
        if target.exists():target.unlink()
        shutil.copy2(root/rel,target)
    # The proxy is the frozen qualified fixture, not concurrent worktree edits.
    for rel in('tests/xui/xui_test_proxy.c','tests/xui/xui_test_proxy.h','tests/xui/xui_flow_graph_widget_test.c'):
        inputs[str(baseline/rel)]=sha(baseline/rel)
    for folder in('include','extlibs/xrt','src/xui/core','src/xge/core'):
        for p in(source/folder).rglob('*.h'):inputs[str(p)]=sha(p)
    report=dict(complete=False,passed=False,scope=__doc__,inputs=inputs,rows=[],
        shippingDelivery=False,nativeSdkRebuilt=False,networkTests=False,source=str(source))
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    env=dict(os.environ)
    key='PATH'if win else'LD_LIBRARY_PATH';env[key]=str(runtime)+os.pathsep+env.get(key,'')
    def run(name,command):
        command=list(map(str,command));start=time.monotonic()
        result=subprocess.run(command,cwd=source,env=env,capture_output=True,timeout=240)
        log=out/(name+'.log');log.write_bytes(result.stdout+result.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=result.returncode,
            seconds=time.monotonic()-start,logSha256=sha(log)))
        save();print(name,result.returncode,flush=True);assert not result.returncode,log
        return result.stdout+result.stderr
    flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-parameter',
        '-Wno-cast-function-type','-DXGE_XRT_HOST=1','-DXGE_STATIC_BUILD=1',
        '-ffunction-sections','-fdata-sections','-include',sdk/'xge_build_config.h']
    flags+=['-I'+str(p)for p in(source,source/'include',source/'src/xui/core',source/'src/xge/core',source/'tests/xui')]
    systems=['-l'+name for name in native['system_libraries']]+list(native['external_link_libraries'])
    if not win:systems+=['-Wl,-rpath,'+str(runtime)]
    library=runtime/('xlc.dll'if win else'libxlc.so')
    try:
        for opt in('O0','O2'):
            units=[source/'src/xui/workflow'/name for name in('xui_flow_graph.c','xui_workflow.c','xui_flow_graph_widget.c')]
            units+=[source/'tests/xui/xui_test_proxy.c']
            for label,case in(('shared','xui_flow_graph_shared_widget_test.c'),('legacy','xui_flow_graph_widget_test.c')):
                binary=out/(label+'-'+opt+('.exe'if win else''))
                run(label+'-'+opt+'-build',[cc,*flags,'-'+opt,*units,source/'tests/xui'/case,archive,library,
                    '-Wl,--gc-sections',*systems,'-o',binary])
                assert b'passed'in run(label+'-'+opt+'-run',[binary]).lower()
        report.update(complete=True,inputsUnchanged=all(sha(p)==h for p,h in inputs.items()))
        report['passed']=report['inputsUnchanged'];assert report['passed']
    except BaseException as error:report.update(complete=True,error=repr(error));raise
    finally:save()


if __name__=='__main__':main()
