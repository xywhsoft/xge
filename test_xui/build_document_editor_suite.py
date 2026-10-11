#!/usr/bin/env python3
"""Run every permanent lossless Editor entry against one matching actual shared library."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import subprocess
import sys

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source-root',type=Path,default=Path(__file__).resolve().parents[1])
parser.add_argument('--build-dir',type=Path,required=True)
parser.add_argument('--out-dir',type=Path,required=True)
args=parser.parse_args()
if sys.platform not in ('win32','linux'):parser.error('The permanent entries currently support Windows and Linux.')
root,build,out=(p.resolve() for p in (args.source_root,args.build_dir,args.out_dir))
if out.exists():parser.error('--out-dir must be new')
entries=[('move-suite1602','build_document_move_editor_suite.py',1602),
    ('footnote168','build_document_footnote_newline_editor_test.py',168),
    ('deep-prefix672','build_document_deep_prefix_editor_test.py',672),
    ('deep-quote432','build_document_deep_quote_editor_test.py',432),
    ('nested-list432','build_document_nested_list_editor_test.py',432),
    ('nonparagraph3024','build_document_nonparagraph_prefix_editor_test.py',3024),
    ('html-rowspan2','build_document_html_rowspan_editor_test.py',2)]
library=build/('xge.dll' if os.name=='nt' else 'libxge.so')
def sha(path):
    with Path(path).open('rb') as stream:return hashlib.file_digest(stream,'sha256').hexdigest()
paths={Path(__file__).resolve(),library,build/'xge_build_config.h'}
paths.update(root/'test_xui'/script for _,script,_ in entries)
if os.name=='nt':paths.add(build/'xge.lib')
missing=[str(p) for p in paths if not p.is_file()]
if missing:parser.error('Missing inputs: '+', '.join(missing))
inputs={str(p):sha(p) for p in paths};out.mkdir(parents=True)
report={'status':'running','source_root':str(root),'build_dir':str(build),'input_sha256':inputs,
    'library_sha256':sha(library),'entries':[],'subreports':[],'actual_product_checks':0,
    'editor_cases':0,'geometry_width_cases':0,'backend':'proxy','physical_platform_acceptance':False,
    'full_document_acceptance':False}
def save():
    (out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
def validate_child(folder,cases):
    path=folder/'report.json';child=json.loads(path.read_text(encoding='utf-8'))
    assert child.get('status','passed')=='passed' and child['inputs_unchanged']
    assert child['editor_cases']==cases and child['backend']=='proxy'
    assert child['library_sha256']==report['library_sha256']
    assert len(child['checks'])==2
    assert all(c['actual_exit']==0 and sha(c['log'])==c['log_sha256'] for c in child['checks'])
    child_inputs=folder/'inputs.json';assert sha(child_inputs)==child['inputs_sha256']
    assert all(sha(p)==h for p,h in json.loads(child_inputs.read_text(encoding='utf-8')).items())
    if cases==2:
        assert child['geometry_width_cases']==2 and child['geometry_widths']==[240,640]
        assert child['html_and_native_clipboard_shared_history'] and child['merged_geometry_lower_row_hit_and_draw_requests']
        assert not child['actual_system_clipboard_ime_or_pixels_accepted']
    return {'path':str(path),'sha256':sha(path),'editor_cases':cases,'actual_product_checks':2,
        'geometry_width_cases':2 if cases==2 else 0}
save()
try:
    for name,script,cases in entries:
        assert all(sha(p)==h for p,h in inputs.items())
        command=[sys.executable,str(root/'test_xui'/script),'--source-root',str(root),
            '--build-dir',str(build),'--out-dir',str(out/name)]
        log=out/(name+'.log')
        with log.open('x',encoding='utf-8') as stream:
            result=subprocess.run(command,cwd=root,stdout=stream,stderr=subprocess.STDOUT)
        entry={'name':name,'command':command,'actual_exit':result.returncode,'log':str(log),'log_sha256':sha(log)}
        report['entries'].append(entry);save()
        print(name,'entry actual EXIT',result.returncode,flush=True)
        if result.returncode:print(log.read_text(encoding='utf-8',errors='replace')[-7000:],flush=True);raise SystemExit(result.returncode)
        child_path=out/name/'report.json';child=json.loads(child_path.read_text(encoding='utf-8'))
        if cases==1602:
            assert child['status']=='passed' and child['editor_cases']==1602 and child['actual_product_checks']==10
            assert child['backend']=='proxy' and child['library_sha256']==report['library_sha256']
            assert all(sha(p)==h for p,h in child['input_sha256'].items())
            assert len(child['entries'])==5
            for row in child['entries']:
                assert row['actual_exit']==0 and sha(out/name/(row['name']+'.log'))==row['log_sha256']
            descendants=[('block',252),('block-kind',243),('sole-child',945),('complex',81),('code-tab',81)]
        else:descendants=[('',cases)]
        products=[validate_child(out/name/descendant,n) for descendant,n in descendants]
        report['subreports'].extend(products)
        entry.update({'report_sha256':sha(child_path),'editor_cases':cases,
            'actual_product_checks':sum(p['actual_product_checks'] for p in products)})
        report['editor_cases']+=cases;report['actual_product_checks']+=entry['actual_product_checks']
        report['geometry_width_cases']+=sum(p['geometry_width_cases'] for p in products)
        save()
    assert len(report['entries'])==7 and len(report['subreports'])==11
    assert report['editor_cases']==6332 and report['actual_product_checks']==22 and report['geometry_width_cases']==2
    assert all(sha(p)==h for p,h in inputs.items())
    report['inputs_unchanged']=True;report['status']='passed';save()
except BaseException as error:
    report['status']='failed';report['error']=str(error);save();raise
print('Permanent Document Editor suite:6332 cases,22 actual product checks and2 HTML geometry widths passed;proxy backend.',flush=True)
