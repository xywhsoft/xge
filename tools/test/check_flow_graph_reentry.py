"""Source-unit FlowGraph Busy gate against an exact canonical XRT/compiler cohort.

Copies an immutable native baseline, overlays only this change, and excludes
unrelated working-tree geometry. Does not claim a rebuilt SDK or UI acceptance.
"""
from pathlib import Path
import argparse,hashlib,json,os,re,shutil,subprocess,time


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in('root','baseline-source','runtime-dir','compiler-report','config-dir','out'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();root,baseline,runtime,build,config,out=[v.resolve()for v in(a.root,a.baseline_source,a.runtime_dir,a.compiler_report,a.config_dir,a.out)]
    assert not out.is_relative_to(root)and not out.is_relative_to(baseline)
    sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();win=os.name=='nt';gcc=Path(shutil.which('gcc')).resolve()
    metadata=json.loads(build.read_text());assert metadata['complete']and metadata['passed']and metadata['fullTccXrtAndVfs']and not metadata['changedInputs']
    for name in(('xl.exe','xl.dll','xlc.dll')if win else('xl','libxl.so','libxlc.so')):assert sha(runtime/name)==metadata['products'][name]['sha256']
    assert sha(baseline/'extlibs/xrt/xrt.h')==sha(Path(metadata['source'])/'lib/xrt/xrt.h')
    overlays=['include/xui.h','src/xui/core/xui_internal.h','src/xui/workflow/xui_flow_graph.c','src/xui/workflow/xui_flow_graph_widget.c','src/xui/workflow/xui_workflow.c','tests/xui/xui_flow_graph_config_reentry_test.c','tests/xui/xui_flow_graph_busy_probes.inc','tests/xui/xui_workflow_busy_probes.inc']
    inputs={str(root/r):sha(root/r)for r in overlays};inputs.update({str(f):sha(f)for f in(Path(__file__).resolve(),build,config/'xge_build_config.h',gcc,runtime/('xlc.dll'if win else'libxlc.so'))})
    out.mkdir(parents=True,exist_ok=False);source=out/'source'
    def link(src,dst):
        try:os.link(src,dst);return dst
        except OSError:return shutil.copy2(src,dst)
    shutil.copytree(baseline,source,copy_function=link)
    (source/'build').mkdir(exist_ok=True)
    for rel in overlays:
        target=source/rel;target.parent.mkdir(parents=True,exist_ok=True);assert target.resolve().is_relative_to(source.resolve())
        if target.exists():target.unlink()
        shutil.copy2(root/rel,target);assert sha(target)==inputs[str(root/rel)]
    # Physical signature coverage, rather than an unmaintained handpicked list.
    header=(source/'include/xui.h').read_text(encoding='utf-8-sig');body=(source/'src/xui/workflow/xui_flow_graph.c').read_text(encoding='utf-8-sig')
    public={m[1]for m in re.finditer(r'XUI_API int (xuiFlowGraph\w+)\(xui_flow_graph pGraph\b[^;]*\);',header)}
    probes=(source/'tests/xui/xui_flow_graph_busy_probes.inc').read_text();assert public==set(re.findall(r'assert\((xuiFlowGraph\w+)\(',probes))and len(public)==63
    for name in public:
        match=re.search(r'XUI_API int '+name+r'\([^;{}]*\)\s*\{\s*if \(__xuiFlowBusy\(pGraph\)\) return XUI_ERROR_BUSY;',body);assert match,name
    assert body.count('xrtValueRelease(')==1 and body.count('xrtValueDeepClone(')==1
    workflows={m[1]for m in re.finditer(r'XUI_API int (xuiWorkflow\w+)\(xui_workflow pWorkflow\b[^;]*\);',header)}
    probes=(source/'tests/xui/xui_workflow_busy_probes.inc').read_text();assert workflows==set(re.findall(r'assert\((xuiWorkflow\w+)\(',probes))
    workflow_body=(source/'src/xui/workflow/xui_workflow.c').read_text(encoding='utf-8-sig')
    for name in workflows:
        assert re.search(r'XUI_API int '+name+r'\([^;{}]*\)\s*\{\s*if \(__xuiWorkflowModelBusy\(pWorkflow\)\) return XUI_ERROR_BUSY;',workflow_body),name
    report=dict(complete=False,passed=False,scope=__doc__,shippingDelivery=False,nativeSdkRebuilt=False,networkTests=False,publicIntEntries=len(public),workflowIntEntries=len(workflows),canonicalXrtSha256=sha(source/'extlibs/xrt/xrt.h'),source=str(source),inputs=inputs,rows=[])
    def save():
        tmp=out/'results.json.tmp';tmp.write_text(json.dumps(report,indent=2)+'\n');os.replace(tmp,out/'results.json')
    temp=out/'tmp';temp.mkdir();env=dict(os.environ,TEMP=str(temp),TMP=str(temp),TMPDIR=str(temp));key='PATH'if win else'LD_LIBRARY_PATH';env[key]=str(runtime)+os.pathsep+env.get(key,'')
    def run(name,command):
        start=time.monotonic();command=list(map(str,command));z=subprocess.run(command,cwd=source,env=env,capture_output=True,timeout=240)
        data=z.stdout+z.stderr;log=out/(name+'.log');log.write_bytes(data);report['rows'].append(dict(name=name,command=command,exitCode=z.returncode,passed=z.returncode==0,seconds=time.monotonic()-start,logSha256=sha(log)));save();print(name,z.returncode,flush=True);assert not z.returncode,log;return data
    flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-parameter','-Wno-unused-function','-Wno-cast-function-type','-DXGE_XRT_HOST=1','-DXRT_USE_SHARED=1','-include',config/'xge_build_config.h']
    flags+=['-I'+str(v)for v in(source,source/'include',source/'src/xui/core',config/'host-xrt')]
    systems=['-lws2_32','-liphlpapi','-lshell32','-lole32','-ladvapi32']if win else['-ldl','-lm','-pthread','-Wl,-rpath,'+str(runtime)]
    model=source/'src/xui/workflow/xui_flow_graph.c';workflow=source/'src/xui/workflow/xui_workflow.c';library=runtime/('xlc.dll'if win else'libxlc.so')
    try:
        for opt in('O0','O2'):
            for label,case,units in[
                ('busy','xui_flow_graph_config_reentry_test.c',[model,workflow]),
                ('model','xui_flow_graph_test.c',[model]),
                ('schema','xui_workflow_schema_test.c',[model,workflow]),
                ('xson','xui_workflow_xson_test.c',[model,workflow])]:
                case=source/'tests/xui'/case
                if label!='busy':inputs[str(baseline/'tests/xui'/case.name)]=sha(baseline/'tests/xui'/case.name)
                exe=out/(label+'-'+opt+('.exe'if win else''));run(label+'-'+opt+'-build',[gcc,*flags,'-'+opt,*units,case,library,*systems,'-o',exe]);raw=run(label+'-'+opt+'-run',[exe])
                if label!='xson':assert b'passed'in raw.lower()
            run('widget-'+opt+'-compile',[gcc,*flags,'-'+opt,'-c',source/'src/xui/workflow/xui_flow_graph_widget.c','-o',out/('widget-'+opt+'.o')])
        report.update(complete=True,passed=True,inputsUnchanged=all(sha(f)==h for f,h in inputs.items()));assert report['inputsUnchanged'];save()
    except BaseException as error:report['error']=repr(error);save();raise


if __name__=='__main__':main()
