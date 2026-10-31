"""Verify the six game-foundation mechanisms, existing 3D paths and core input isolation."""
import argparse
import json
from pathlib import Path
import re
import subprocess
import sys
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/game-foundation';EVIDENCE=ROOT/'artifacts/xge-game-foundation'
FEATURES=['DEBUG_DRAW','MOTION','DYNAMIC','SHADER','BILLBOARD','PARTICLES']
TESTS=('cpu spatial terrain import animation async gpu import_gpu material_gpu sky_gpu light_gpu shadow_gpu ibl_gpu skin_gpu '
       'culling_gpu instancing_gpu terrain_gpu async_gpu fog_gpu foundation_base foundation foundation_gpu local_matrix '
       'mesh_prepare_gpu nodes collision cache core_gpu render_quality_gpu playback playback_gpu light_effects_gpu height_fog_gpu '
       'game_foundation game_foundation_gpu shader_shadow_gpu').split()
def run(args,**kwargs):return subprocess.run([str(x) for x in args],cwd=ROOT,check=True,**kwargs)
def build(profile,name,definitions=()):
    path=OUT/name;run([sys.executable,'tools/build/build.py',profile,path,*[x for d in definitions for x in ['--define',d]]]);return path
def input_check(path):
    exe=path/'test_input_actions.exe';run(['gcc','@config/includes.rsp','-O2','-Wall','-Wextra','-Werror','-DXGE_DLL','-include',path/'xge_build_config.h','tests/xge/test_input_actions.c',path/'xge.lib','-lm','-o',exe]);run([exe])
    report=json.loads((path/'build-report.json').read_text());assert not any('/3d/' in s for s in report['sources'])
    exports=run(['objdump','-p',path/'xge.dll'],capture_output=True,text=True).stdout;assert not re.search(r'\bxge3d\w+',exports)
    assert ('xgeActionsCreate' in exports)==('XGE_ENABLE_INPUT_ACTIONS' in report['features'])
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--profiles',nargs='*',choices=['3d','full-dev'],default=['3d','full-dev']);p.add_argument('--trimming',action='store_true');a=p.parse_args()
    EVIDENCE.mkdir(parents=True,exist_ok=True)
    for name in ['xge-3d','xge-render-quality','xge-animation-playback','xge-light-effects']:(ROOT/'artifacts'/name).mkdir(parents=True,exist_ok=True)
    run([sys.executable,'tests/xge/make_3d_fixtures.py'])
    results={}
    for profile in a.profiles:
        path=build(profile,profile)
        tool=path/'xge3d_ibl.exe';run(['gcc','@config/includes.rsp','-O2','-DXGE_DLL','-include',path/'xge_build_config.h','tools/generate/xge3d_motion/xge3d_ibl.c',path/'xge.lib','-lm','-o',tool])
        run([tool,'artifacts/xge-3d/fixtures/ibl',*(['artifacts/xge-3d/fixtures/white.png']*6),'4','64'])
        run([sys.executable,'tests/xge/test_foundation.py',path,*TESTS]);results[profile]=json.loads((path/'foundation-tests.json').read_text())
        (EVIDENCE/'regression.json').write_text(json.dumps(results,indent=2),encoding='utf-8')
    for enabled in (0,1):input_check(build('core','core-input-'+str(enabled),['XGE_ENABLE_INPUT_ACTIONS='+str(enabled)]))
    disabled=['XGE3D_ENABLE_'+name+'=0' for name in FEATURES]+['XGE_ENABLE_INPUT_ACTIONS=0']
    path=build('3d','six-off',disabled);run([sys.executable,'tests/xge/test_foundation.py',path,'game_foundation','game_foundation_gpu','shader_shadow_gpu','render_quality_gpu'])
    invalid=[['-D'+name+'=2'] for name in ['XGE_ENABLE_INPUT_ACTIONS']+['XGE3D_ENABLE_'+x for x in FEATURES]]
    invalid += [['-DXGE_ENABLE_3D=0','-DXGE3D_ENABLE_'+name+'=1'] for name in FEATURES]
    invalid += [['-DXGE3D_ENABLE_COLLISION=0','-DXGE3D_ENABLE_MOTION=1'],['-DXGE3D_ENABLE_BILLBOARD=0','-DXGE3D_ENABLE_PARTICLES=1']]
    for flags in invalid:
        result=subprocess.run(['gcc','@config/includes.rsp','-E','-P','-x','c',*flags,'-'],input='#include "xge.h"\n',cwd=ROOT,text=True,capture_output=True)
        assert result.returncode and '#error' in result.stderr,flags
    (EVIDENCE/'config-checks.json').write_text(json.dumps({'invalid':invalid,'passed':True,'core_input':[0,1]},indent=2),encoding='utf-8')
    if a.trimming:run([sys.executable,'tests/xge/check_3d_trimming.py',*FEATURES,'BARE','--output-root',OUT/'trimming','--evidence-root',EVIDENCE,'--evidence-prefix','trim'])
    print('Game-foundation verification completed; evidence in',EVIDENCE)
if __name__=='__main__':main()
