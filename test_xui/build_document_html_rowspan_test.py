"""Fresh unified Document kernel and HTML zero-row-span/ownership regression."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import re
import subprocess
import sys

parser=argparse.ArgumentParser()
parser.add_argument('--source-root',type=Path,default=Path(__file__).resolve().parents[1])
parser.add_argument('--out-dir',type=Path,required=True)
parser.add_argument('--variant',choices=['normal','collision'],default='normal')
parser.add_argument('--sanitize',action='store_true')
args=parser.parse_args()
root=args.source_root.resolve();folder=args.out_dir.resolve()
folder.mkdir(parents=True,exist_ok=False)
temporary=folder/'compiler-tmp';temporary.mkdir()
def sha(path):
    with Path(path).open('rb') as stream:return hashlib.file_digest(stream,'sha256').hexdigest()
sources=[]
for line in (root/'xui_document_sources.bat').read_text(encoding='utf-8').splitlines():
    if line.startswith('set XUI_DOCUMENT_SRC='):
        sources.extend(p.replace('\\','/') for p in line.split('=',1)[1].replace('%XUI_DOCUMENT_SRC%','').split())
sources.append('test_xui/xui_test_xrt_impl.c')
assert len(sources)==29 and len(set(sources))==29
tests=['xui_document_html_rowspan_test','xui_document_html_runs_test','xui_document_html_buffer_fault_test','xui_document_html_table_numbers_test','xui_document_html_character_reference_test','xui_document_html_named_reference_test','xui_document_html_markdown_reference_test','xui_document_html_markdown_reference_fault_test','xui_document_html_newline_test']
paths={root/name for name in sources}
paths.update(root.glob('*.h'));paths.update(root.glob('src/*.h'))
paths.update(root.glob('lib/xrt/*.h'));paths.update(root.glob('lib/md4c/*.h'))
paths.update(root.glob('test_xui/*.h'))
paths.update(root/('test_xui/'+name+'.c') for name in tests)
paths.update([root/'lib/md4c/md4c.c',root/'xui_document_sources.bat',Path(__file__).resolve()])
inputs={str(path):sha(path) for path in sorted(paths)}
with (folder/'inputs.json').open('x',encoding='utf-8') as stream:json.dump(inputs,stream,indent=2)
flags=['-std=c11','-O1' if args.sanitize else '-O2','-g','-Wall','-Wextra','-Werror','-Wno-unused-function','-I'+str(root)]
if args.variant=='collision':flags.append('-DXUI_DOC_REFERENCE_HASH_COLLISION_TEST')
if args.sanitize:flags.extend(['-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie'])
links=['-lm','-lws2_32','-liphlpapi'] if os.name=='nt' else ['-lm','-ldl','-lpthread']
env=os.environ.copy();env['TMPDIR']=str(temporary)
if os.name=='nt':env['TMP']=env['TEMP']=str(temporary)
if args.sanitize:
    env['ASAN_OPTIONS']='detect_leaks=1:halt_on_error=1';env['UBSAN_OPTIONS']='halt_on_error=1'
checks=[]
def unchanged():
    assert all(sha(path)==digest for path,digest in inputs.items())
def run(name,command):
    unchanged();command=[str(p) for p in command];log=folder/(name+'.log')
    with log.open('x',encoding='utf-8') as stream:
        result=subprocess.run(command,cwd=root,env=env,stdout=stream,stderr=subprocess.STDOUT)
    checks.append({'name':name,'command':command,'actual_exit':result.returncode,'log':str(log),'log_sha256':sha(log)})
    with (folder/'checks.json').open('w',encoding='utf-8') as stream:json.dump(checks,stream,indent=2)
    assert result.returncode==0,(name,log.read_text(encoding='utf-8',errors='replace'))
    return log.read_text(encoding='utf-8')
objects=[]
for name in sources:
    obj=folder/(Path(name).stem+'.o');objects.append(obj)
    run('compile-'+obj.stem,['gcc',*flags,'-c',root/name,'-o',obj])
archive=folder/'document.a';run('archive',['ar','rcs',archive,*objects])
members=run('archive-members',['ar','t',archive]).splitlines()
assert members==[p.name for p in objects]
for name in tests:
    executable=folder/(name+('.exe' if os.name=='nt' else ''))
    extra=['-DXUI_HTML_ROWSPAN_IMPORT_FAULT_TEST'] if name==tests[0] else []
    run(name+'-compile',['gcc',*flags,*extra,root/('test_xui/'+name+'.c'),archive,*links,'-o',executable])
    output=run(name+'-run',[executable])
    if name==tests[0]:
        assert output.count('independent spans, native/HTML roundtrip, old snapshot and single Undo/Redo passed')==18
        assert '18 literal fixtures, pending-array growth,1024/1025 limits and invalid grid passed' in output
        rows=re.findall(r'HTML rowspan importer OOM ([\w-]+): (\d+) points, null output, retry and zero leaks passed',output)
        assert {name for name,_ in rows}=={'body','two-bodies','nested','pending-growth'} and len(rows)==4
        assert all(int(count)>0 for _,count in rows)
        points=sum(int(count) for _,count in rows)
        assert f'HTML rowspan importer OOM total: {points} points in 4 fixtures passed' in output
    if name==tests[3]:
        assert '22 independent literal cases,0 failures' in output
        assert 'fragment/native/HTML roundtrip, old snapshots and single Undo/Redo passed' in output
    if name==tests[4]:
        assert '22 independent literal cases,0 failures' in output
        assert 'fragment/native/HTML roundtrip, old snapshots and single Undo/Redo passed' in output
        character_faults=re.findall(r'HTML character reference importer buffers: (\d+) allocation failures/null output/retry/zero leaks passed',output)
        assert len(character_faults)==1 and int(character_faults[0])>0
    if name==tests[5]:
        assert '31 independent literal cases,0 failures' in output
        assert 'HTML named references: fragment/native/HTML roundtrip, old snapshots and single Undo/Redo passed' in output
        assert '22310 standard-table/suffix/text-attribute cases passed' in output
        named_faults=re.findall(r'HTML named reference importer buffers: (\d+) allocation failures/null output/retry/zero leaks passed',output)
        assert len(named_faults)==1 and int(named_faults[0])>0
    if name==tests[6]:
        assert '24 independent raw-source goldens and96 metadata-preserving root/quote backtick/tilde resource commands passed' in output
        assert '201 code-block cases;36 exact accepted conversions/insertions and165 atomic refusals, native/source/shared history checked' in output
    if name==tests[7]:
        assert 'Fence info faults:4 exhaustive4096-byte language OOM/cancel scans passed' in output
        info_faults=re.findall(r'Fence info fault insert=(\d) cancel=(\d): (\d+) points;',output)
        assert len(info_faults)==4 and {(a,b) for a,b,_ in info_faults}=={('0','0'),('0','1'),('1','0'),('1','1')}
        assert all(int(n)>0 for _,_,n in info_faults)
    if name==tests[8]:
        assert '26 independent literal goldens, fragment/native/HTML roundtrip, old snapshot and one Undo/Redo passed' in output
        assert '78 cross-profile cases,18 strict lossless conversions/insertions,60 atomic refusals passed' in output
        newline_faults=re.findall(r'HTML newline buffers:(\d+) importer/exporter allocation failures, null outputs, retry and zero leaks passed',output)
        assert len(newline_faults)==1 and int(newline_faults[0])>0
    print(output.splitlines()[-1],flush=True)
unchanged()
with (folder/'report.json').open('x',encoding='utf-8') as stream:
    json.dump({'platform':sys.platform,'variant':args.variant,'sanitizers':args.sanitize,
        'inputs_sha256':sha(folder/'inputs.json'),'checks':checks,'fresh_kernel_units':29,'reused_kernel_units':0,
        'literal_fixtures':18,'numeric_literal_fixtures':22,'numeric_fragment_native_html_and_single_history':True,'character_reference_literal_fixtures':22,'character_reference_buffer_fault_points':int(character_faults[0]),'named_reference_literal_fixtures':31,'named_reference_table_context_cases':22310,'named_reference_buffer_fault_points':int(named_faults[0]),'cross_profile_literal_cases':201,'cross_profile_raw_goldens':24,'cross_profile_resource_commands':96,'cross_profile_lossless_cases':36,'cross_profile_atomic_refusals':165,'long_info_fault_scans':4,'long_info_fault_points':sum(int(n) for _,_,n in info_faults),'newline_literal_cases':26,'newline_cross_profile_cases':78,'newline_lossless_cases':18,'newline_atomic_refusals':60,'newline_buffer_fault_points':int(newline_faults[0]),'fault_fixtures':4,'fault_points':points,'library_sha256':sha(archive),
        'object_sha256':{str(path):sha(path) for path in objects},'inputs_unchanged':True,
        'status':'passed','source_landed':False,'full_document_acceptance':False},stream,indent=2)
print('Fresh HTML rowspan entry passed:',folder/'report.json',flush=True)
