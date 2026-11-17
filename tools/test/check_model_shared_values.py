"""Qualify explicit shared model values and preserve native Clone behavior.

CPU source units: exact retained identities, original Clone isolation, command
history snapshots, no-allocation shared graph updates/queries, owned/raw model
lifetimes, shared value Drop Busy, rollback and all existing model gates.
No public language data-snapshot contract or SDK delivery is implied.
"""
from pathlib import Path
import argparse,hashlib,json,os,re,shutil,subprocess,time


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','baseline-source','runtime-dir','compiler-report','config-dir','out'):
        p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();root,baseline,runtime,build,config,out=[v.resolve() for v in
        (a.root,a.baseline_source,a.runtime_dir,a.compiler_report,a.config_dir,a.out)]
    assert not out.is_relative_to(root) and not out.is_relative_to(baseline)
    sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest()
    win=os.name=='nt';gcc=Path(shutil.which('gcc')).resolve()
    metadata=json.loads(build.read_text(encoding='utf8'))
    assert metadata['complete'] and metadata['passed'] and metadata['fullTccXrtAndVfs'] and not metadata['changedInputs']
    for name in (('xl.exe','xl.dll','xlc.dll') if win else ('xl','libxl.so','libxlc.so')):
        assert sha(runtime/name)==metadata['products'][name]['sha256']
    assert sha(baseline/'extlibs/xrt/xrt.h')==sha(Path(metadata['source'])/'lib/xrt/xrt.h')
    overlays=['tests/xui/xui_workflow_shared_storage_test.c','tests/xui/xui_model_shared_values_test.c',
        'tests/xui/xui_workflow_callbacks_owned_test.c','tests/xui/xui_workflow_xson_shared_test.c',
        'src/xui/core/xui_internal.h','src/xui/workflow/xui_flow_graph.c',
        'tests/xui/xui_workflow_value_reentry_test.c','include/xui.h','src/xui/workflow/xui_workflow.c','src/xui/workflow/xui_workflow_owned.inl',
        'tests/xui/xui_workflow_owned_test.c','tests/xui/xui_workflow_busy_probes.inc',
        'tests/xui/xui_workflow_schema_test.c','tests/xui/xui_workflow_xson_test.c',
        'tests/xui/xui_flow_graph_owned_test.c','tests/xui/xui_flow_graph_config_reentry_test.c',
        'tests/xui/xui_flow_graph_busy_probes.inc']
    inputs={str(root/r):sha(root/r) for r in overlays}
    inputs.update((str(f),sha(f)) for f in (Path(__file__).resolve(),build,config/'xge_build_config.h',gcc,
        runtime/('xlc.dll' if win else 'libxlc.so')))
    out.mkdir(parents=True,exist_ok=False);source=out/'source'
    def link(src,dst):
        try:os.link(src,dst);return dst
        except OSError:return shutil.copy2(src,dst)
    shutil.copytree(baseline,source,copy_function=link)
    for rel in overlays:
        target=source/rel;target.parent.mkdir(parents=True,exist_ok=True)
        assert target.resolve().is_relative_to(source)
        if target.exists():target.unlink()
        shutil.copy2(root/rel,target)
    text=(source/'src/xui/workflow/xui_workflow.c').read_text(encoding='utf8')
    entries=re.findall(r'^XUI_API int (xuiWorkflow\w+)\(xui_workflow pWorkflow\b[^;{}]*\)\s*\{',text,re.M)
    assert len(entries)==35
    probes=(source/'tests/xui/xui_workflow_busy_probes.inc').read_text(encoding='utf8')
    assert set(entries)==set(re.findall(r'assert\((xuiWorkflow\w+)\(',probes))
    for name in entries:
        if name=='xuiWorkflowDestroyChecked':continue
        assert re.search(r'XUI_API int '+name+r'\([^;{}]*\)\s*\{\s*if \(__xuiWorkflowModelBusy\(pWorkflow\)\) return XUI_ERROR_BUSY;\s*xui_workflow_operation_t __ownedCall=\{0\};',text),name
    baseline_paths=['src/xui/workflow/xui_flow_graph.c','src/xui/workflow/xui_flow_graph_owned.inl',
        'src/xui/workflow/xui_workflow.c','src/xui/workflow/xui_workflow_owned.inl',
        'src/xge/core/xge_data_lifetime.inl','src/xge/core/xge_data_lifetime_internal.h']
    inputs.update((str(baseline/r),sha(baseline/r)) for r in baseline_paths)
    report=dict(complete=False,passed=False,scope=__doc__,shippingDelivery=False,nativeSdkRebuilt=False,
        networkTests=False,graphicsDeviceTests=False,protectedExistingOperations=34,
        canonicalXrtSha256=sha(source/'extlibs/xrt/xrt.h'),source=str(source),inputs=inputs,rows=[])
    def save():
        tmp=out/'results.json.tmp';tmp.write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');os.replace(tmp,out/'results.json')
    (out/'build').mkdir()
    temp=out/'tmp';temp.mkdir();env=dict(os.environ,TEMP=str(temp),TMP=str(temp),TMPDIR=str(temp))
    key='PATH' if win else 'LD_LIBRARY_PATH';env[key]=str(runtime)+os.pathsep+env.get(key,'')
    def run(name,command,reject=False):
        command=list(map(str,command));start=time.monotonic()
        z=subprocess.run(command,cwd=out,env=env,capture_output=True,timeout=180)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=z.returncode,
            passed=(bool(z.returncode)==reject),expectedRejection=reject,seconds=time.monotonic()-start,logSha256=sha(log)))
        save();print(name,z.returncode,flush=True);assert bool(z.returncode)==reject,log
    flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-parameter','-Wno-unused-function',
        '-Wno-cast-function-type','-DXGE_XRT_HOST=1','-DXRT_USE_SHARED=1','-include',config/'xge_build_config.h']
    flags+=['-I'+str(v) for v in (source,source/'include',source/'src/xui/core',source/'src/xge/core')]
    systems=['-lws2_32','-liphlpapi','-lshell32','-lole32','-ladvapi32'] if win else ['-ldl','-lm','-pthread','-Wl,-rpath,'+str(runtime)]
    units=[source/'src/xui/workflow/xui_flow_graph.c',source/'src/xui/workflow/xui_workflow.c']
    library=runtime/('xlc.dll' if win else 'libxlc.so')
    try:
        baseline_exe=out/('baseline-value-reentry'+('.exe' if win else ''))
        run('baseline-build',[gcc,*flags,'-O2','-DXUI_TEST_BASELINE_NO_SHARED=1',baseline/'src/xui/workflow/xui_flow_graph.c',
            baseline/'src/xui/workflow/xui_workflow.c',source/'tests/xui/xui_workflow_value_reentry_test.c',
            library,*systems,'-o',baseline_exe])
        run('baseline-rejects',[baseline_exe],reject=True)
        assert b'xuiWorkflowDestroyChecked(workflow)==XUI_ERROR_BUSY' in (out/'baseline-rejects.log').read_bytes()
        for opt in ('O0','O2'):
            for label,case in [('workflow-storage','xui_workflow_shared_storage_test.c'),('workflow-callbacks','xui_workflow_callbacks_owned_test.c'),
                ('workflow-xson-shared','xui_workflow_xson_shared_test.c'),
                ('model-shared','xui_model_shared_values_test.c'),('workflow-values','xui_workflow_value_reentry_test.c'),('workflow-owned','xui_workflow_owned_test.c'),('graph-owned','xui_flow_graph_owned_test.c'),
                ('graph-busy','xui_flow_graph_config_reentry_test.c'),('schema','xui_workflow_schema_test.c'),('xson','xui_workflow_xson_test.c')]:
                exe=out/(label+'-'+opt+('.exe' if win else ''))
                run(label+'-'+opt+'-build',[gcc,*flags,'-'+opt,*units,source/'tests/xui'/case,library,*systems,'-o',exe])
                run(label+'-'+opt+'-run',[exe])
                if label in ('schema','xson'):
                    # The unchanged scenario also runs against actual shared
                    # origins, exercising nested public calls and file reload.
                    shared=out/(label+'-shared.c')
                    shared.write_text((source/'tests/xui'/case).read_text(encoding='utf8').replace(
                        'xuiWorkflowCreate(', 'xuiWorkflowCreateOwned('),encoding='utf8')
                    exe=out/(label+'-shared-'+opt+('.exe' if win else ''))
                    run(label+'-shared-'+opt+'-build',[gcc,*flags,'-'+opt,*units,shared,library,*systems,'-o',exe])
                    run(label+'-shared-'+opt+'-run',[exe])
            # Header and internal widget callers must still compile; no full
            # archive replacement is pretended by this source-unit gate.
            run('workflow-widget-'+opt+'-compile',[gcc,*flags,'-'+opt,'-c',source/'src/xui/workflow/xui_workflow_widget.c','-o',out/('widget-'+opt+'.o')])
        header=out/'headers.c';header.write_text('#include "xge.h"\n#include "xui.h"\n',encoding='utf8')
        for standard,language in (('c11','c'),('c++11','c++'),('c++17','c++')):
            run('headers-'+standard,[gcc,'-std='+standard,'-Wall','-Wextra','-Werror','-x',language,
                '-I'+str(source),'-I'+str(source/'include'),'-fsyntax-only',header])
        report.update(complete=True,passed=True,inputsUnchanged=all(sha(f)==h for f,h in inputs.items()))
        assert report['inputsUnchanged'];save()
    except BaseException as error:
        report['error']=repr(error);save();raise


if __name__=='__main__':main()
