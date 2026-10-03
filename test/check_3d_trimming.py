"""Audit every optional 3D module against its actual DLL and consumer header."""
import argparse
import json
from pathlib import Path
import re
import subprocess
import sys
ROOT=Path(__file__).resolve().parents[1]
MODULES=['MODEL','LIGHTING','SHADOW','IBL','ANIMATION','TERRAIN','ASYNC','FOG']
def run(args,**kwargs):return subprocess.run([str(a) for a in args],cwd=ROOT,check=True,**kwargs)
def audit(name):
    out=ROOT/'build'/f'p6-3d-{name.lower()}-off'
    definitions=['XGE3D_ENABLE_'+n+'=0' for n in MODULES] if name=='BARE' else ['XGE3D_ENABLE_'+name+'=0']
    run([sys.executable,'tools/build_profile.py','3d',out,*[arg for d in definitions for arg in ['--define',d]]])
    report=json.loads((out/'build-report.json').read_text());features=set(report['features'])
    manifest=json.loads((ROOT/'tools/features.json').read_text())
    for source,guard in manifest['3d_sources'].items():
        assert (source in report['sources'])==(guard in features),(name,source,guard)
    assert not any('ufbx' in s or 'xge3d_motion' in s for s in report['sources'])
    text=run(['objdump','-p',out/'xge.dll'],capture_output=True,text=True).stdout
    exports=set(re.findall(r'^\s*\[\s*\d+\].*\s+(xge3d\w+)\s*$',text,re.M))
    args=['gcc','-O2','-Wall','-Wextra','-Werror','-Wno-missing-field-initializers','-DXGE_DLL','-DXUI_DLL','-I.',
          '-include',out/'xge_build_config.h']
    header=run([*args,'-E','-P','-x','c','-'],input='#include "xge.h"\n',capture_output=True,text=True).stdout
    declarations=set(re.findall(r'\b(xge3d\w+)\s*\(',header));assert declarations==exports,(name,declarations-exports,exports-declarations)
    representatives={'MODEL':'xge3dModelLoad','LIGHTING':'xge3dNodeSetLight','SHADOW':'xge3dShadowDefault','IBL':'xge3dEnvironmentSetIBL',
                     'ANIMATION':'xge3dAnimatorCreate','TERRAIN':'xge3dTerrainCreate','ASYNC':'xge3dLoaderCreate','FOG':'xge3dFogDefault'}
    for module,symbol in representatives.items():assert (symbol in exports)==('XGE3D_ENABLE_'+module in features),(name,symbol)
    imports=re.findall(r'DLL Name: (.+)',text);assert not any('stdc++' in s.lower() or 'harfbuzz' in s.lower() for s in imports)
    tests=['cpu','gpu']
    if 'XGE3D_ENABLE_MODEL' in features:tests+=['import','import_gpu']
    if 'XGE3D_ENABLE_ANIMATION' in features:tests+=['animation']
    if 'XGE3D_ENABLE_ASYNC' in features:tests+=['async','async_gpu']
    if 'XGE3D_ENABLE_TERRAIN' in features:tests+=['terrain']
    for test in tests:
        exe=out/f'test_3d_{test}.exe';run([*args,'test/test_3d_'+test+'.c',out/'xge.lib','-lm','-o',exe]);run([exe])
    run([sys.executable,'tools/build_profile.py','3d',out,*[arg for d in definitions for arg in ['--define',d]]])
    hot=json.loads((out/'build-report.json').read_text());assert not hot['compiled'] and not hot['linked']
    summary={'case':name,'defines':definitions,'features':sorted(features),'sources':report['sources'],'exports':sorted(exports),
             'runtime_bytes':report['runtime_bytes'],'imports':imports,'tests':tests,'hot_seconds':hot['seconds']}
    (ROOT/f'artifacts/xge-3d/p6-trim-{name.lower()}.json').write_text(json.dumps(summary,indent=2))
    print(name,report['runtime_bytes'],'B;',len(exports),'matching declarations/exports; actual tests',','.join(tests),flush=True)
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('modules',nargs='*',default=[*MODULES,'BARE']);a=p.parse_args()
    if any(name not in [*MODULES,'BARE'] for name in a.modules):p.error('modules must be '+', '.join([*MODULES,'BARE']))
    # Generate once before tests, rather than rewriting shared fixtures between
    # cases while another independent profile may be reading them.
    run([sys.executable,'test/make_3d_fixtures.py'])
    for name in a.modules:audit(name)
if __name__=='__main__':main()
