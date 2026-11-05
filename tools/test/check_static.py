"""CPU-only whole-profile link and direct single-XRT static archive checks."""
from pathlib import Path
import argparse,hashlib,json,os,re,shutil,subprocess,sys,time

def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('root','build','xlang-root','out'):p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--import-kinds-only',action='store_true',help='Read-only real System32 export kind/forwarder and unknown-admission tests')
    a=p.parse_args();root,build,xlang,out=[v.resolve()for v in(a.root,a.build,a.xlang_root,a.out)]
    if out.exists():p.error('Use a new evidence directory')
    out.mkdir(parents=True);metadata=json.loads((build/'build-report.json').read_text());assert metadata['passed']and metadata['complete']
    archive=build/metadata['archive'];assert sha(archive)==metadata['archive_sha256']
    if a.import_kinds_only:
        assert os.name=='nt'
        sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'build'))
        import static_support
        classifier=static_support.WindowsExportKinds();checks=[]
        for symbol,expected in [('_fpreset','function'),('_HUGE','data'),('_iob','data'),('labs','function')]:
            actual=classifier.classify('msvcrt.dll',symbol)
            assert actual['known']and actual['kind']==expected,(symbol,actual)
            checks.append(dict(name=symbol,expected=expected,evidence=actual))
        image=classifier.image('msvcrt.dll')
        ordinal=image['base']+image['names']['_HUGE']
        actual=classifier.classify('msvcrt.dll',None,ordinal)
        assert actual['known']and actual['kind']=='data'
        checks.append(dict(name='real-ordinal',ordinal=ordinal,evidence=actual))
        image=classifier.image('kernel32.dll');forward=None
        for symbol,index in image['names'].items():
            rva=image['table'][index]
            if image['export']<=rva<image['export']+image['exportSize']:
                result=classifier.classify('kernel32.dll',symbol)
                if result['known']and len(result['chain'])>1:forward=result;break
        assert forward,'No resolved real forwarder was tested'
        checks.append(dict(name='real-forwarder',evidence=forward))
        for name,value in [('missing-export',classifier.classify('msvcrt.dll','__absent_static_kind_test')),
            ('invalid-path',classifier.classify('../kernel32.dll','Sleep')),
            ('cycle',classifier.classify('msvcrt.dll','labs',seen=(('msvcrt.dll','labs',None),)))]:
            assert not value['known']and value['kind']=='unknown'
            checks.append(dict(name=name,evidence=value))
        classification=metadata['compiler_support']['importClassification']
        assert not classification['unknownInventory']
        changed=[]
        for library in metadata['compiler_support']['archives']:
            for row in library['importMembers']:
                assert row['kindKnown']and row['kind']in('function','data')
                if row['name']=='_fpreset':assert row['kind']=='function';changed.append(row)
        assert changed
        (out/'results.json').write_text(json.dumps(dict(passed=True,scope='Read-only System32 PE kind proof; no LoadLibrary, callbacks, generated PE or Native recompilation.',checks=checks,
            importInventoryKnown=True,fpresetProof=changed,dllInputs=classifier.files,
            inputs={str(Path(static_support.__file__)):sha(static_support.__file__),str(Path(__file__)):sha(__file__)}),indent=2)+'\n')
        print(out/'results.json');return
    mapping={}
    for name,module in re.findall(r'XL6_ARTIFACT_NATIVE_SYMBOL\("([^"\n]+)",\s*(NULL|"[^"\n]+")\)',(xlang/'src/compile/xl6_artifact_native_symbols.inc').read_text()):
        mapping.setdefault(name,set())
        if module!='NULL':mapping[name].add(module.strip('"'))
    modules=sorted(set().union(*(mapping[v]for v in metadata['xrt_undefined_symbols'])))
    xrt=out/'xrt.c';xrt.write_text(''.join('#define '+v+' 1\n'for v in modules)+'#define XRT_IMPLEMENTATION\n#include "'+(xlang/'lib/xrt/xrt.h').as_posix()+'"\n')
    main=out/'main.c';main.write_text('#include "xge.h"\n#include "xui.h"\n#include "xge_static_xrt_contract.h"\n#include <assert.h>\n#include <stdio.h>\nint main(void){xge_build_info_t info={0};info.size=sizeof(info);assert(xgeGetBuildInfo(&info)==0);assert(xgeColorRGBA(1,2,3,4)==XGE_COLOR_RGBA(1,2,3,4));xge_data_object data=0;assert(xgeDataObjectCreateOwned(&data)==0&&data);bool consumed=false;assert(xgeDataObjectReleaseOwnerChecked(data,&consumed)==0&&consumed);const xge_host_xrt_contract_t local=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;assert(local.dataModel==xgeStaticXrtContractV1.dataModel);assert(local.coreLayout==xgeStaticXrtContractV1.coreLayout);printf("FILE=%zu;CRT-owned IO local;CPU PASS\\n",sizeof(FILE));return 0;}\n')
    results=[];commands=[];cc=shutil.which('gcc');windows=os.name=='nt';suffix='.exe'if windows else''
    def run(name,args):
        start=time.monotonic();r=subprocess.run([str(v)for v in args],capture_output=True);log=out/(name+'.log');log.write_bytes(r.stdout+r.stderr)
        results.append(dict(name=name,exitCode=r.returncode,seconds=round(time.monotonic()-start,3),logSha256=sha(log)));commands.append([str(v)for v in args])
        (out/'results.json').write_text(json.dumps(dict(passed=all(v['exitCode']==0 for v in results),results=results,commands=commands),indent=2)+'\n')
        if r.returncode:raise RuntimeError(str(log))
        return r.stdout.decode(errors='replace')
    flags=['-O2','-ffunction-sections','-fdata-sections','-DXL6_NATIVE_LINK_SCHEMA=1','-DXGE_XRT_HOST=1','-include',build/'xge_build_config.h','-I'+str(build),'-I'+str(root/'include'),'-I'+str(root)]
    if not windows:flags+=['-D_GNU_SOURCE']
    run('compile-xrt',[cc,*flags,'-c',xrt,'-o',out/'xrt.o'])
    run('compile-main',[cc,'-std=c11','-Wall','-Wextra','-Werror',*flags,'-c',main,'-o',out/'main.o'])
    header=out/'header.cpp';header.write_text('#include \"xge.h\"\n#include \"xui.h\"\n#include \"xge_static_xrt_contract.h\"\nstatic_assert(sizeof(xgeStaticXrtContractV1)>0,\"contract\");\n')
    run('header-cpp',[shutil.which('g++'),'-std=c++17','-Wall','-Wextra','-Werror',*flags,'-fsyntax-only',header])
    for mode in ('whole','gc'):
        link=[cc,out/'main.o',out/'xrt.o','-Wl,--whole-archive',archive,'-Wl,--no-whole-archive']if mode=='whole'else[cc,out/'main.o',out/'xrt.o',archive,'-Wl,--gc-sections']
        # GCC normal CRT/startup is intentionally used. TCC pairing is a separate actual-link gate.
        link+=['-Wl,-Map='+str(out/(mode+'.map')),*['-l'+v for v in metadata['system_libraries']],*[v['path']for v in metadata.get('external_link_libraries',[])],'-o',out/(mode+suffix)]
        run('link-'+mode,link);run('run-'+mode,[out/(mode+suffix)])
        imports=run('imports-'+mode,['objdump','-p',out/(mode+suffix)])
        if re.search(r'(?:DLL Name:|NEEDED).*xge',imports,re.I):raise RuntimeError('Unexpected dynamic XGE dependency')
    run('archive-defined',['nm','-g','--defined-only',archive])
    if re.search(r'\b(?:__imp_)?xrt[A-Z]\w*\s*$',(out/'archive-defined.log').read_text(),re.M):raise RuntimeError('Archive embeds XRT')
    final=json.loads((out/'results.json').read_text());final.update(scope=__doc__,passed=True,modules=modules,archiveSha256=sha(archive),buildReportSha256=sha(build/'build-report.json'),binaryBytes={v:(out/(v+suffix)).stat().st_size for v in ('whole','gc')},compiler=[cc,sha(cc)],notes=['Whole-archive/no-GC link checks all selected native members against one program XRT and normal GCC CRT.','No window, GPU, browser or OS clipboard operation is run.','TCC import/CRT compatibility is separately qualified, not implied by the GNU link.'])
    (out/'results.json').write_text(json.dumps(final,indent=2)+'\n');print(str(out/'results.json'))
if __name__=='__main__':main()
