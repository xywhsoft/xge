"""HTML-only merged-table paste, native copy/history and visual geometry on one actual product."""
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
if sys.platform not in ('win32','linux'):parser.error('This entry currently supports Windows and Linux GCC.')
root,build,folder=(p.resolve() for p in (args.source_root,args.build_dir,args.out_dir))
if folder.exists():parser.error('--out-dir must be new')
library=build/('xge.dll' if os.name=='nt' else 'libxge.so')
config=build/'xge_build_config.h';driver=root/'test_xui/xui_document_html_rowspan_editor_test.c'
def sha(path):
    with Path(path).open('rb') as stream:return hashlib.file_digest(stream,'sha256').hexdigest()
paths={Path(__file__).resolve(),library,config,driver,root/'src/xui_unicode.c',root/'test_xui/xui_test_proxy.c'}
paths.update(root.glob('*.h'));paths.update(root.glob('src/*.h'));paths.update(root.glob('lib/xrt/*.h'));paths.update(root.glob('test_xui/*.h'))
if os.name=='nt':paths.add(build/'xge.lib')
missing=[str(p) for p in paths if not p.is_file()]
if missing:parser.error('Missing inputs: '+', '.join(missing))
inputs={str(p):sha(p) for p in sorted(paths)}
folder.mkdir(parents=True)
with (folder/'inputs.json').open('x',encoding='utf-8') as stream:json.dump(inputs,stream,indent=2)
env=os.environ.copy();key='PATH' if os.name=='nt' else 'LD_LIBRARY_PATH'
env[key]=str(build)+os.pathsep+env.get(key,'');env['TMPDIR']=str(folder)
if os.name=='nt':env['TMP']=env['TEMP']=str(folder)
flags=['-std=c11','-O2','-g','-Wall','-Wextra','-Werror','-Wno-unused-function','-DXGE_DLL','-DXUI_DLL','-I'+str(root),'-I'+str(root/'test_xui'),'-include',str(config)]
links=([build/'xge.lib','-lm','-lws2_32','-liphlpapi'] if os.name=='nt' else [library,'-lm','-ldl','-lpthread'])
checks=[]
def unchanged():
    assert all(sha(path)==digest for path,digest in inputs.items()),'A protected input changed.'
def run(name,command):
    unchanged();command=[str(p) for p in command];log=folder/(name+'.log')
    with log.open('x',encoding='utf-8') as stream:
        result=subprocess.run(command,cwd=root,env=env,stdout=stream,stderr=subprocess.STDOUT)
    checks.append({'name':name,'command':command,'actual_exit':result.returncode,'log':str(log),'log_sha256':sha(log)})
    (folder/'checks.json').write_text(json.dumps(checks,indent=2),encoding='utf-8')
    unchanged();text=log.read_text(encoding='utf-8',errors='replace')
    if result.returncode:print(text[-7000:],flush=True);raise SystemExit(result.returncode)
    return text
executable=folder/('html-rowspan-editor.exe' if os.name=='nt' else 'html-rowspan-editor')
run('compile',['gcc',*flags,driver,root/'test_xui/xui_test_proxy.c',root/'src/xui_unicode.c',*links,'-o',executable])
text=run('clipboard-and-geometry',[executable])
assert text.count('HTML-only Paste, row-group spans, Copy/native Paste and one-step Undo/Redo passed; proxy backend')==2
assert 'HTML rowspan Editor: two clipboard/history cases passed; proxy backend' in text
assert 'HTML rowspan Editor geometry:240/640 widths, merged cell bounds, lower-row hit and actual draw requests passed; proxy backend' in text
with (folder/'report.json').open('x',encoding='utf-8') as stream:
    json.dump({'platform':sys.platform,'checks':checks,'inputs_sha256':sha(folder/'inputs.json'),
        'library_sha256':sha(library),'executable_sha256':sha(executable),'editor_cases':2,
        'geometry_width_cases':2,'geometry_widths':[240,640],'actual_product_checks':2,
        'html_and_native_clipboard_shared_history':True,'merged_geometry_lower_row_hit_and_draw_requests':True,
        'inputs_unchanged':True,'backend':'proxy','physical_platform_acceptance':False,
        'actual_system_clipboard_ime_or_pixels_accepted':False,'full_document_acceptance':False,'status':'passed'},stream,indent=2)
print('HTML rowspan Editor:2 clipboard/history cases and2 widths passed;proxy backend.',flush=True)
