#!/usr/bin/env python3
"""Measure the public kernel and actual XGE Editor without freezing its worker."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import time

sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
tests = Path(__file__).resolve().parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source-root', type=Path, default=tests.parent)
parser.add_argument('--build-dir', type=Path, required=True)
parser.add_argument('--out-dir', type=Path, required=True)
parser.add_argument('--font', type=Path)
parser.add_argument('--headless', action='store_true', help='Linux Xvfb/Mesa software GL')
args = parser.parse_args()
windows = sys.platform=='win32'
if not windows and sys.platform!='linux':parser.error('Current native entry supports Windows/Linux')
if args.headless and windows:parser.error('--headless is Linux-only')
root, build, out = (p.resolve() for p in (args.source_root,args.build_dir,args.out_dir))
font = args.font or (Path(os.environ.get('WINDIR',r'C:\Windows'))/'Fonts/arial.ttf' if windows else Path('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf'))
font = font.resolve()
library = build/('xge.lib' if windows else 'libxge.so')
runtime = build/('xge.dll' if windows else 'libxge.so')
config = build/'xge_build_config.h'
source = tests/'xui_document_native_scale_test.c'
compiler = shutil.which(os.environ.get('CC','gcc'))
if not compiler:parser.error('C compiler not found')
if args.headless and not shutil.which('xvfb-run'):parser.error('xvfb-run is required')
paths = [Path(__file__).resolve(),source,font,config,library,runtime]
missing=[str(p) for p in paths if not p.is_file()]
if missing:parser.error('Missing inputs: '+', '.join(missing))
if out.exists():parser.error('--out-dir must be new to preserve previous results')
inputs={str(p):sha(p) for p in paths}
out.mkdir(parents=True)
report={'platform':platform.platform(),'machine':platform.node(),'processor':platform.processor(),
        'started_at':time.time(),'input_sha256':inputs,'headless_software_gl':args.headless,
        'worker_frozen':False,'timing_is_diagnostic':True,'checks':[],'status':'running',
        'physical_ime_dpi_readers_verified':False,'fixed_machine_release_gate_verified':False}
env=os.environ.copy()
key='PATH' if windows else 'LD_LIBRARY_PATH'
env[key]=str(build)+os.pathsep+env.get(key,'')
def save():
    (out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
def run(name,command):
    log=out/(name+'.log')
    with log.open('x',encoding='utf-8') as stream:
        try:
            result=subprocess.run([str(p) for p in command],cwd=root,env=env,
                stdout=stream,stderr=subprocess.STDOUT,timeout=300)
            code=result.returncode
        except subprocess.TimeoutExpired:code=124
    report['checks'].append({'name':name,'command':[str(p) for p in command],
                            'actual_exit':code,'log_sha256':sha(log)})
    save();print(name,'actual EXIT',code,flush=True)
    text=log.read_text(encoding='utf-8',errors='replace')
    if code:
        report['status']='failed';save();print(text[-6000:],flush=True);raise SystemExit(code)
    return text
executable=out/('document-native-scale.exe' if windows else 'document-native-scale')
links=[library,'-lm','-lopengl32'] if windows else [library,'-lm','-lGL','-Wl,-rpath,'+str(build)]
run('compile',[compiler,'-std=c11','-O2','-Wall','-Wextra','-Werror','-DXGE_DLL','-DXUI_DLL',
    '-Wno-unused-function','-include',config,'-I'+str(root),source,*links,'-MD','-MF',out/'document.d','-o',executable])
command=[executable,out,font]
if args.headless:command=['xvfb-run','-a','-s','-screen 0 1024x768x24','env','LIBGL_ALWAYS_SOFTWARE=1',*command]
try:
    text=run('run',command)
    if 'Native scale acceptance: Rich/100KiB core plus six actual 1/10MiB Editor cases passed;' not in text:
        raise RuntimeError('Missing final complete acceptance marker')
    if 'Actual view caret viewport diagnostics: 0 out-of-viewport checks' not in text:
        raise RuntimeError('Actual widget caret must remain inside the viewport')
    caret_lines=[line for line in text.splitlines() if line.startswith('Actual view caret ') and 'outside=' in line]
    if len(caret_lines)!=12 or any('outside=0' not in line for line in caret_lines):
        raise RuntimeError('All twelve actual caret viewport checks are required')
    for mib in [1,10]:
        for mode in ['visual','source','live']:
            if f'Actual Editor {mib}MiB {mode}: first=' not in text:
                raise RuntimeError('Missing native Editor case')
    if not all(sha(Path(p))==h for p,h in inputs.items()):raise RuntimeError('Input changed during run')
    report['images']=[]
    for mib in [1,10]:
        for mode in ['visual','source','live']:
            path=out/f'editor-{mib}MiB-{mode}.png'
            if path.read_bytes()[:8]!=b'\x89PNG\r\n\x1a\n':raise RuntimeError('Invalid PNG')
            report['images'].append({'name':path.name,'sha256':sha(path)})
    report['executable_sha256']=sha(executable);report['status']='passed';save()
except Exception:
    report['status']='failed';save();raise
print(text.rstrip(),flush=True)
