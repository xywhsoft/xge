"""Document export-buffer ownership, pointer identity, OOM and core regressions.

Compiles the complete isolated document-core group and its real XRT provider.
UI consumer changes additionally require the matched full-engine UI gate.
"""
from pathlib import Path
import argparse,hashlib,json,os,shlex,shutil,subprocess,time

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('root','freeze','out'):p.add_argument('--'+n,type=Path,required=True)
    p.add_argument('--reuse',type=Path)
    p.add_argument('--sanitize',action='store_true');a=p.parse_args()
    root,freeze,out=[v.resolve()for v in(a.root,a.freeze,a.out)];sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    manifest=json.loads(freeze.read_text());assert all(sha(root/n)==h for n,h in manifest['files'].items())
    assert not(a.sanitize and os.name=='nt');out.mkdir(parents=True,exist_ok=False)
    (out/'build/document').mkdir(parents=True)
    cc,cpp=[Path(shutil.which(v)).resolve()for v in ('gcc','g++')]
    toolchain={str(v):sha(v)for v in (cc,cpp)}
    reuse=json.loads(a.reuse.read_text())if a.reuse else None
    if reuse:
        assert reuse['freezeSha256']==sha(freeze) and reuse['toolchain']==toolchain
    report=dict(complete=False,passed=False,scope=__doc__,shippingDelivery=False,root=str(root),freeze=str(freeze),freezeSha256=sha(freeze),scriptSha256=sha(__file__),toolchain=toolchain,objects=[],programs=[],rows=[])
    save=lambda:(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,args,timeout=240):
        args=list(map(str,args));actual=args
        if args[0]in(str(cc),str(cpp)):
            rsp=out/(name+'.rsp');rsp.write_text('\n'.join('"'+v.replace('\\','/').replace('"','\\"')+'"'for v in args[1:])+'\n');actual=[args[0],'@'+str(rsp)]
        if name.endswith('xui_document_test-execute'):timeout=1200
        start=time.monotonic();z=subprocess.run(actual,cwd=out,capture_output=True,timeout=timeout);log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        report['rows'].append(dict(name=name,command=args,exitCode=z.returncode,seconds=time.monotonic()-start,logSha256=sha(log)));save();print(name,z.returncode,flush=True);assert not z.returncode,log
    try:
        import sys
        sources=subprocess.check_output([sys.executable,'-B',str(root/'tools/build/build.py'),'--source-group','document-core'],cwd=root,text=True).strip().split()
        sources+=['tests/xui/xui_test_xrt_impl.c']
        include=['-I'+str(root/v[2:])for v in shlex.split((root/'config/includes.rsp').read_text())if v.startswith('-I')]+['-I'+str(root/'tests')]
        flags=['-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-parameter','-DXRT_MODULE_MEMORY_DEBUG',*include]
        sanitizer=['-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer']if a.sanitize else[]
        systems=['-lws2_32','-liphlpapi','-lm']if os.name=='nt'else['-pthread','-ldl','-lm']
        for opt in ('O0','O2'):
            folder=out/opt;folder.mkdir();objects=[]
            for relative in sources:
                source=root/relative;name=source.stem;obj=folder/(name+'.o');objects.append(obj)
                command=list(map(str,[cc,*flags,'-'+opt,*sanitizer,'-c',source,'-o',obj]))
                old=next((v for v in reuse['objects']if Path(v['source'])==source and Path(v['path']).parent.name==opt),None)if reuse else None
                if old:
                    receipt=next(v for v in reuse['rows']if v['name']==opt+'-'+name)
                    assert receipt['exitCode']==0 and receipt['command'][:-1]==command[:-1] and receipt['command'][-1]==old['path']
                    assert sha(old['path'])==old['sha256']and sha(source)==old['sourceSha256']
                    shutil.copy2(old['path'],obj);report['rows'].append(dict(name=opt+'-'+name,exitCode=0,reused=True,parent=str(a.reuse.resolve()),parentSha256=sha(a.reuse),objectSha256=old['sha256'],command=command));save();print(opt+'-'+name,'verified reuse',flush=True)
                else:run(opt+'-'+name,command)
                report['objects'].append(dict(path=str(obj),sha256=sha(obj),source=str(source),sourceSha256=sha(source)))
            for name in ('xui_document_export_allocator_test','xui_document_test','xui_document_html_character_reference_test','xui_document_html_named_reference_test'):
                program=folder/(name+('.exe'if os.name=='nt'else''))
                wrap=['-Wl,--wrap=xrtJsonWriterTake']if name=='xui_document_export_allocator_test'else[]
                run(opt+'-'+name+'-build',[cc,*flags,'-'+opt,*sanitizer,root/'tests/xui'/(name+'.c'),*objects,*systems,*wrap,'-o',program]);run(opt+'-'+name+'-execute',[program])
                report['programs'].append(dict(path=str(program),sha256=sha(program),bytes=program.stat().st_size))
        for standard,tool,language in (('c11',cc,'c'),('c++11',cpp,'c++'),('c++17',cpp,'c++')):
            run('header-'+standard,[tool,'-std='+standard,'-Wall','-Wextra','-Werror',*include,'-x',language,'-fsyntax-only',root/'include/xui_document.h'])
        report.update(complete=True,passed=True,inputsUnchanged=all(sha(root/n)==h for n,h in manifest['files'].items())and all(sha(p)==h for p,h in toolchain.items())and all(sha(v['path'])==v['sha256']for v in report['objects']));assert report['inputsUnchanged']
    except Exception as e:report.update(complete=True,passed=False,error=str(e));raise
    finally:save()

if __name__=='__main__':main()
