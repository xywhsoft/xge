"""CPU Workflow canvas qualification with exact static SDK and model units.

Normal cases use production code; the explicitly named linker-wrap case also
injects an unconsumed cleanup status. No graphics/network operation, SDK rebuild
or normal runtime installation is implied.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,time


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','baseline-source','static-sdk','runtime-dir','compiler-report','proxy-fixtures','out'):
        p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();root,baseline,sdk,runtime,build,fixtures,out=[v.resolve() for v in
        (a.root,a.baseline_source,a.static_sdk,a.runtime_dir,a.compiler_report,a.proxy_fixtures,a.out)]
    sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest()
    win=os.name=='nt';cc=Path(shutil.which('gcc')).resolve()
    native_report=sdk/'build-report.json';native=json.loads(native_report.read_text(encoding='utf8'))
    metadata=json.loads(build.read_text(encoding='utf8'));assert native['complete'] and native['passed']
    assert native['xrt_mode']=='direct-external' and metadata['complete'] and metadata['passed']
    assert metadata['fullTccXrtAndVfs'] and not metadata['changedInputs']
    archive=sdk/native['archive'];assert sha(archive)==native['archive_sha256']
    assert sha(baseline/'extlibs/xrt/xrt.h')==native['xrt_header_sha256']==sha(Path(metadata['source'])/'lib/xrt/xrt.h')
    products=[runtime/n for n in (('xl.exe','xl.dll','xlc.dll') if win else ('xl','libxl.so','libxlc.so'))]
    for f in products:assert sha(f)==metadata['products'][f.name]['sha256']
    proxy_pins={'xui_test_proxy.c':'740c86aee90b1373324909c74d3ae6c9f446b0c52c84c3ea84cea141c53bdc48',
        'xui_test_proxy.h':'b42cd48cd68966d969575f717ade56f357dacbcabce746730c9bfb275dd0a80a'}
    # These are the previously qualified CPU fixture bytes, not the concurrent
    # geometry changes in this worktree. Retain an exact source hash per file.
    for n,h in proxy_pins.items():assert sha(fixtures/'tests/xui'/n)==h
    overlays=['include/xui.h','src/xui/core/xui_internal.h','src/xui/workflow/xui_workflow.c',
        'src/xui/workflow/xui_workflow_owned.inl','src/xui/workflow/xui_workflow_widget.c',
        'src/xui/workflow/xui_flow_graph_widget.c','src/xui/workflow/xui_flow_graph_widget_internal.h',
        'tests/xui/xui_workflow_shared_widget_test.c','tests/xui/xui_workflow_widget_test.c',
        'tests/xui/xui_workflow_interaction_test.c','tests/xui/xui_flow_graph_shared_widget_test.c',
        'tests/xui/xui_flow_graph_widget_test.c']
    inputs={str(f):sha(f) for f in (Path(__file__).resolve(),cc,build,native_report,archive,sdk/'xge_build_config.h',*products)}
    inputs.update((str(root/r),sha(root/r)) for r in overlays)
    inputs.update((str(fixtures/'tests/xui'/n),h) for n,h in proxy_pins.items())
    out.mkdir(parents=True,exist_ok=False);source=out/'source'
    assert not out.is_relative_to(root) and not out.is_relative_to(baseline)
    def link(src,dst):
        try:os.link(src,dst);return dst
        except OSError:return shutil.copy2(src,dst)
    shutil.copytree(baseline,source,copy_function=link)
    def copy(src,target):
        target.parent.mkdir(parents=True,exist_ok=True)
        if target.exists():target.unlink()
        shutil.copy2(src,target)
    for r in overlays:copy(root/r,source/r)
    for n in proxy_pins:copy(fixtures/'tests/xui'/n,source/'tests/xui'/n)
    for folder in ('include','extlibs/xrt','src/xui/core','src/xge/core'):
        inputs.update((str(f),sha(f)) for f in (source/folder).rglob('*.h'))
    report=dict(complete=False,passed=False,scope=__doc__,source=str(source),inputs=inputs,rows=[],
        shippingDelivery=False,nativeSdkRebuilt=False,networkTests=False,graphicsDeviceTests=False,
        xrtSha256=native['xrt_header_sha256'])
    def save():
        tmp=out/'results.json.tmp';tmp.write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');os.replace(tmp,out/'results.json')
    env=dict(os.environ);key='PATH' if win else 'LD_LIBRARY_PATH'
    env[key]=str(runtime)+os.pathsep+env.get(key,'')
    def run(name,command):
        command=list(map(str,command));start=time.monotonic()
        z=subprocess.run(command,cwd=out,env=env,capture_output=True,timeout=180)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=z.returncode,passed=z.returncode==0,
            seconds=time.monotonic()-start,logSha256=sha(log)))
        save();print(name,z.returncode,flush=True);assert not z.returncode,log
    flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-parameter',
        '-Wno-cast-function-type','-DXGE_XRT_HOST=1','-DXGE_STATIC_BUILD=1','-ffunction-sections','-fdata-sections',
        '-include',sdk/'xge_build_config.h']
    flags+=['-I'+str(f) for f in (source,source/'include',source/'src/xui/core',source/'src/xge/core',source/'tests/xui')]
    systems=['-l'+n for n in native['system_libraries']]+list(native['external_link_libraries'])
    if not win:systems+=['-Wl,-rpath,'+str(runtime)]
    library=runtime/('xlc.dll' if win else 'libxlc.so')
    units=[source/'src/xui/workflow/xui_workflow.c',source/'src/xui/workflow/xui_workflow_widget.c',
        source/'src/xui/workflow/xui_flow_graph_widget.c',source/'tests/xui/xui_test_proxy.c']
    try:
        for opt in ('O0','O2'):
            for label,case in [('workflow-shared','xui_workflow_shared_widget_test.c'),
                ('workflow-legacy','xui_workflow_widget_test.c'),('workflow-interaction','xui_workflow_interaction_test.c'),
                ('graph-shared','xui_flow_graph_shared_widget_test.c'),('graph-legacy','xui_flow_graph_widget_test.c')]:
                binary=out/(label+'-'+opt+('.exe' if win else ''))
                injection=['-Wl,--wrap=xuiWorkflowReleaseOwnerChecked','-Wl,--wrap=xuiWidgetInvalidate'] if label=='workflow-shared' else []
                run(label+'-'+opt+'-build',[cc,*flags,'-'+opt,*units,source/'tests/xui'/case,archive,library,
                    '-Wl,--gc-sections',*injection,*systems,'-o',binary])
                run(label+'-'+opt+'-run',[binary])
        header=out/'headers.c';header.write_text('#include "xge.h"\n#include "xui.h"\n',encoding='utf8')
        for standard,language in (('c11','c'),('c++11','c++'),('c++17','c++')):
            run('headers-'+standard,[cc,'-std='+standard,'-Wall','-Wextra','-Werror','-x',language,
                '-I'+str(source),'-I'+str(source/'include'),'-fsyntax-only',header])
        report.update(complete=True,passed=True,inputsUnchanged=all(sha(f)==h for f,h in inputs.items()))
        assert report['inputsUnchanged'];save()
    except BaseException as error:
        report['error']=repr(error);save();raise


if __name__=='__main__':main()
