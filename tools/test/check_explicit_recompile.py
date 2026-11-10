"""Reject unsafe qualified-object reuse at the actual build CLI.

The caller supplies an original full build, its immutable source and a derived
candidate built with explicit TU selection. No parent/source is modified.
"""
from pathlib import Path
import argparse,hashlib,json,shutil,subprocess,sys

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','parent','parent-source','derived','out'):
        p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();root,parent,source,derived,out=[v.resolve() for v in
        (a.root,a.parent,a.parent_source,a.derived,a.out)]
    sha=lambda path:hashlib.sha256(Path(path).read_bytes()).hexdigest()
    metadata=json.loads((parent/'build-report.json').read_text())
    result=json.loads((derived/'build-report.json').read_text())
    assert metadata['linked'] and not metadata.get('qualified_object_build')
    proof=result['qualified_object_build']
    assert result['linked'] and proof['inputs_unchanged'] and len(proof['recompiled'])==1
    assert proof['recompiled'][0]['source']=='single/xge.c'
    out.mkdir(parents=True,exist_ok=False)
    files=[Path(__file__).resolve(),parent/'build-report.json',derived/'build-report.json']
    files.extend((root/'tools/build').glob('*.py'))
    inputs={str(v):sha(v) for v in files}
    report=dict(passed=False,inputs=inputs,rows=[],positiveBuild=str(derived/'build-report.json'))
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    def reject(name,extra,message,tree=root,base=parent):
        args=[sys.executable,'-B',tree/'tools/build/build.py','default',out/name,
            '--host-xrt','--relink-from',base,'--relink-source',source,*extra]
        args=list(map(str,args));r=subprocess.run(args,cwd=tree,capture_output=True,timeout=90)
        log=out/(name+'.log');log.write_bytes(r.stdout+r.stderr)
        report['rows'].append(dict(name=name,command=args,exitCode=r.returncode,logSha256=sha(log)))
        save();assert r.returncode and message.encode() in log.read_bytes(),log
        assert not (out/name/'build-report.json').exists();print(name,'rejected',flush=True)
    reject('unknown',['--recompile-source','unknown.c'],'not part of the qualified parent')
    reject('duplicate',['--recompile-source','single/xge.c','--recompile-source','single/xge.c'],'Duplicate --recompile-source')
    reject('configuration',['--define','XGE_ENABLE_2D=0'],'Qualified relink configuration differs')
    reject('derived-parent',['--recompile-source','single/xge.c'],'original full build',base=derived)
    # Ordinary copying preserves parent bytes/stamps without shared writeable
    # hardlinks. Deliberately change a TU outside the explicitly allowed set.
    candidate=out/'undeclared-source'
    shutil.copytree(source,candidate,ignore=shutil.ignore_patterns('build','.git','__pycache__'))
    shutil.copytree(root/'tools/build',candidate/'tools/build',dirs_exist_ok=True)
    changed=candidate/'src/xui/core/xui_core.c'
    changed.write_bytes(changed.read_bytes()+b'\n/* undeclared negative test */\n')
    reject('undeclared',['--recompile-source','src/xui/controls/xui_button.c'],
        'Qualified relink dependency content differs',tree=candidate)
    command=[sys.executable,'-B',root/'tools/build/build.py','default',out/'no-parent','--recompile-source','single/xge.c']
    r=subprocess.run(list(map(str,command)),cwd=root,capture_output=True,timeout=30)
    log=out/'no-parent.log';log.write_bytes(r.stdout+r.stderr)
    report['rows'].append(dict(name='no-parent',command=list(map(str,command)),exitCode=r.returncode,logSha256=sha(log)))
    assert r.returncode and b'requires an explicit qualified relink parent' in log.read_bytes()
    report.update(passed=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()))
    assert report['inputsUnchanged'];save();print('explicit source rebuild qualification passed',flush=True)

if __name__=='__main__':main()
